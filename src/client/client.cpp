#include "client.h"

#include "server.h"

// Check if the position of a player is valid
bool Client::CheckPosition(Vec3 &newPosition, double &newStance) {
	player->previousPosition = player->position;
	/*
	Vec3 testPos {
		0,0,0
	};
	AABB testBox {
		Vec3 {
			48,0,48
		},
		Vec3 {
			64,128,64
		}
	};
	*/
	// Accept what the player has sent us
	player->position = newPosition;
	player->stance = newStance;

	// Player has collided
	/*
	if (player->CheckCollision(testPos,testBox)) {
		player->position = player->position + player->CheckPushback(testPos,testBox);
		return false;
	}
	*/
	return true;
}

// Set the client up to receive another packet
ssize_t Client::Setup() {
	// Set stuff up for the next batch of packets
	offset = 0;
	previousOffset = 0;

	// Read Data
	return read(clientFd, message, PACKET_MAX);
}

// Print the received data
void Client::PrintReceived(ssize_t bytes_received, Packet packetType) {
	std::string debugMessage = "";
	if (debugReceivedPacketType) {
		debugMessage += "Received " + PacketIdToLabel(packetType) + " from " + player->username + "! (" + std::to_string(bytes_received) + " Bytes)";
	}
	if (debugReceivedBytes) {
		debugMessage += "\n" + Uint8ArrayToHexDump(message,bytes_received);
	}
	if (debugReceivedPacketType || debugReceivedBytes) {
		Betrock::Logger::Instance().Debug(debugMessage);
	}
}

// Handle the latest server-bound packet from the client
void Client::HandlePacket() {
	//auto serverTime = Betrock::Server::Instance().GetServerTime();
	//int64_t lastPacketTime = serverTime;
	bool validPacket = true;
	// Prep for next packet
	ssize_t bytes_received = Setup();

	// If we receive no Data from the player, such as when they
	// crash or close the game without quitting
	if (bytes_received <= 0) {
		DisconnectClient("No data",true,false);
		return;
	}

	//lastPacketTime = serverTime;

	// Read packet bundle until end
	if (debugReceivedBundleDelimiter) {
		Betrock::Logger::Instance().Debug("--- Start of Packet bundle ---");
	}
	while (validPacket && offset < bytes_received && GetConnectionStatus() > ConnectionStatus::Disconnected) {
		uint8_t packetIndex = EntryToByte(message,offset);
		Packet packetType = (Packet)packetIndex;

		// Provide debug info
		if (debugReceivedBytes || debugReceivedPacketType) {
			PrintReceived(bytes_received,packetType);
		}

		// Legacy ping
		if (packetIndex == 0xFE) {
			uint8_t p1 = EntryToByte(message,offset);
			uint8_t p2 = EntryToByte(message,offset);
			if (p1 == 0x01 && p2 == 0xFA) {
				HandleLegacyPing();
				SetConnectionStatus(ConnectionStatus::Disconnected);
				break;
			}
		}

		// Get the current Dimension
		// TODO: We probably don't need to run this on every single packet!
		World* world = Betrock::Server::Instance().GetWorld(player->dimension);
		
		// Ensure proper packet order
		if ((packetType == Packet::Handshake && GetConnectionStatus() == ConnectionStatus::Handshake) ||
			(packetType == Packet::LoginRequest && GetConnectionStatus() == ConnectionStatus::LoggingIn) ||
			GetConnectionStatus() == ConnectionStatus::Connected
		)
		{
			switch(packetType) {
				case Packet::KeepAlive:
					HandleKeepAlive();
					break;
				case Packet::LoginRequest:
					HandleLoginRequest(world);
					break;
				case Packet::Handshake:
					HandleHandshake();
					break;
				case Packet::ChatMessage:
					HandleChatMessage();
					break;
				case Packet::UseEntity:
					HandleUseEntity();
					break;
				case Packet::Respawn:
					HandleRespawn();
					break;
				case Packet::Player:
					HandlePlayerGrounded();
					break;
				case Packet::PlayerPosition:
					HandlePlayerPosition();
					break;
				case Packet::PlayerLook:
					HandlePlayerLook();
					break;
				case Packet::PlayerPositionLook:
					HandlePlayerPositionLook();
					break;
				case Packet::HoldingChange:
					HandleHoldingChange();
					break;
				case Packet::Animation:
					HandleAnimation();
					break;
				case Packet::EntityAction:
					HandleEntityAction();
					break;
				case Packet::PlayerDigging:
					HandlePlayerDigging(world);
					break;
				case Packet::PlayerBlockPlacement:
					HandlePlayerBlockPlacement(world);
					break;
				case Packet::CloseWindow:
					HandleCloseWindow();
					break;
				case Packet::WindowClick:
					HandleWindowClick();
					break;
				case Packet::UpdateSign:
					HandleUpdateSign();
					break;
				case Packet::Disconnect:
					HandleDisconnect();
					break;
				default:
					Betrock::Logger::Instance().Debug("Unhandled Server-bound packet: " + std::to_string(packetIndex) + "\n" + Uint8ArrayToHexDump(message,bytes_received));
					validPacket = false;
					break;
			}
			if (player != nullptr && GetConnectionStatus() == ConnectionStatus::Connected) {
				if (debugPlayerStatus) {
					player->PrintStats();
				}
				// TODO: Fix this from killing the player during lag
				// Kill player if he goes below 0,0
				/*
				if (client.player->position.y < 0) {
					Respond::UpdateHealth(client.response,0);
				}
				*/
			}
		} else {
			SetConnectionStatus(ConnectionStatus::Disconnected);
		}
	}
	SendNewChunks();

	BroadcastToClients(broadcastResponse);
	BroadcastToClients(broadcastOthersResponse, this);
	SendResponse(true);
	
	if (debugNumberOfPacketBytes) {
		Betrock::Logger::Instance().Debug("--- " + std::to_string(offset) + "/" + std::to_string(bytes_received) + " Bytes Read from Packet ---"); 
	}
}

// Handle each Client on their own thread
// As long as the client stays connected, this function stays running
// It also creates the player object that is used for everything
void Client::HandleClient() {
  	auto &server = Betrock::Server::Instance();
	player = std::make_unique<Player>(
		server.GetLatestEntityId(),
		Int3ToVec3(server.GetSpawnPoint()),
		server.GetSpawnDimension(),
		// TODO: Maybe set these later?
		server.GetSpawnWorld(),
		Int3ToVec3(server.GetSpawnPoint()),
		server.GetSpawnDimension(),
		server.GetSpawnWorld()
	);
	ClearInventory();

	// While the player is connected, read packets from them
	while (server.IsAlive() && GetConnectionStatus() > ConnectionStatus::Disconnected) {
		HandlePacket();
		// A packet is handled every tick
		std::this_thread::sleep_for(std::chrono::milliseconds(1000/TICK_SPEED));
	}
	
	close(clientFd);
	clientFd = -1;

	// If the server is dead, it'll take care of all this
	if (server.IsAlive()) {
		player->Save();
		{
			std::scoped_lock lockConnectedClients(server.GetConnectedClientMutex());
			auto &clients = server.GetConnectedClients();
			auto it = std::find(clients.begin(), clients.end(), shared_from_this());
			if (it != clients.end()) {
				clients.erase(it);
			}
		}
	}
}

// Update the position of oneself for other clients
bool Client::UpdatePositionForOthers([[maybe_unused]] bool includeLook) {
	Respond::EntityTeleport(
		broadcastOthersResponse,
		player->entityId,
		Vec3ToEntityInt3(player->position),
		ConvertFloatToPackedByte(player->yaw),
		ConvertFloatToPackedByte(player->pitch)
	);
	player->previousPosition = player->position;
	/*
	if (GetEuclidianDistance(player->position,player->lastTickPosition) > 4.0) {
		Respond::EntityTeleport(
			broadcastOthersResponse,
			player->entityId,
			Vec3ToEntityInt3(player->position),
			ConvertFloatToPackedByte(player->yaw),
			ConvertFloatToPackedByte(player->pitch)
		);
		player->lastTickPosition = player->position;
	} else {
		if (includeLook) {
			Respond::EntityRelativeMove(
				broadcastOthersResponse,
				player->entityId,
				Vec3ToEntityInt3(player->position-player->lastTickPosition)
			);
		} else {
			Respond::EntityLookRelativeMove(
				broadcastOthersResponse,
				player->entityId,
				Vec3ToEntityInt3(player->position-player->lastTickPosition),
				ConvertFloatToPackedByte(player->yaw),
				ConvertFloatToPackedByte(player->pitch)
			);
		}
	}*/
	return true;
}

// Check if the block position is intersecting with the player
bool Client::BlockTooCloseToPosition(Int3 position) {
    // Player's bounding box
    double playerMinX = player->position.x - 0.3;
    double playerMaxX = player->position.x + 0.3;

    double playerMinY = player->position.y;
	double playerMaxY = player->position.y + 1.8;
	if (player->crouching) {
		playerMaxY = player->position.y + 1.5;  // Correct: modifies the existing variable
	}

    double playerMinZ = player->position.z - 0.25;
    double playerMaxZ = player->position.z + 0.25;

    // Block's bounding box (aligned to integer grid)
    double blockMinX = static_cast<double>(position.x);
    double blockMaxX = blockMinX + 1.0;

    double blockMinY = static_cast<double>(position.y);
    double blockMaxY = blockMinY + 1.0;

    double blockMinZ = static_cast<double>(position.z);
    double blockMaxZ = blockMinZ + 1.0;

    // Check for overlap on all three axes
    bool overlapX = playerMinX < blockMaxX && playerMaxX > blockMinX;
    bool overlapY = playerMinY < blockMaxY && playerMaxY > blockMinY;
    bool overlapZ = playerMinZ < blockMaxZ && playerMaxZ > blockMinZ;

    // If all axes overlap, the bounding boxes intersect
    return overlapX && overlapY && overlapZ;
}

// This should be used for disconnecting clients
// Disconnect the current client from the server
void Client::DisconnectClient(std::string disconnectMessage, bool tellOthers, bool tellPlayer) {
	SetConnectionStatus(ConnectionStatus::Disconnected);
	if (tellPlayer) {
		Respond::Disconnect(disconnectMessage);
		SendResponse(true);
	}
	// Inform other clients
	if (tellOthers) {
		Respond::ChatMessage(broadcastOthersResponse, "§e" + player->username + " left the game.");
	}
	Respond::DestroyEntity(broadcastOthersResponse,player->entityId);
	BroadcastToClients(broadcastOthersResponse,this);
	Betrock::Logger::Instance().Info(player->username + " has disconnected. (" + disconnectMessage + ")");
}

// Add something to the current clients upcoming response packet
void Client::AppendResponse(std::vector<uint8_t> &addition) {
	if (!addition.empty()) {
		std::lock_guard<std::mutex> lock(responseMutex);
		response.insert(response.end(), addition.begin(), addition.end());
	}
}

// Send the contents of the response packet to the Client
void Client::SendResponse(bool autoclear) {
	std::lock_guard<std::mutex> lock(responseMutex);
	if (GetConnectionStatus() <= ConnectionStatus::Disconnected) {
		return;
	}
	if (response.empty()) {
		Respond::KeepAlive(response);
	}

	std::string debugMessage = "";
	if (debugSentPacketType) {
		debugMessage += "Sending " + PacketIdToLabel((Packet)response[0]) + " to " + player->username + "(" + std::to_string(player->entityId) + ") ! (" + std::to_string(response.size()) + " Bytes)";
	}
	if (debugSentBytes) {
		if (((Packet)response[0] == Packet::Chunk || (Packet)response[0] == Packet::PreChunk) && debugDisablePrintChunk) {
			// Do nothing
		} else {
			debugMessage += "\n" + Uint8ArrayToHexDump(&response[0],response.size());
		}
	}
	if (debugSentPacketType || debugSentBytes) {
		Betrock::Logger::Instance().Debug(debugMessage);
	}
	
	ssize_t bytes_sent = send(clientFd, response.data(), response.size(), 0);
	if (bytes_sent == -1) {
		perror("send");
		return;
	}
	if (autoclear) {
		response.clear();
	}
}

// Teleport the client to the requested coordinate
void Client::Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw, float pitch) {
    player->position = position;
	player->position.y += STANCE_OFFSET;
    player->yaw = yaw;
    player->pitch = pitch;
    player->stance = position.y;
    newChunks.clear();
	//SendResponse(true);
    DetermineVisibleChunks(true);
    Respond::PlayerPositionLook(response, player.get());
}

// Teleport the client to the requested while keeping view
void Client::TeleportKeepView(std::vector<uint8_t> &response, Vec3 position) {
    player->position = position;
	player->position.y += STANCE_OFFSET;
    player->stance = position.y;
    newChunks.clear();
	//SendResponse(true);
    DetermineVisibleChunks(true);
    Respond::PlayerPosition(response, player.get());
}

// Respawn the Client by sending them back to spawn
void Client::Respawn(std::vector<uint8_t> &response) {
    player->dimension = player->spawnDimension;
    player->world = player->spawnWorld;
    Teleport(response, player->spawnPosition);
    Respond::Respawn(response, player->dimension);
    // After respawning, the health is automatically set back to the maximum health
    // The Client should do this automatically
    player->health = HEALTH_MAX;
	// If keep inventory is on, preserve player inventory
	if (!keepInventory) {
		ClearInventory();
	} else {
		UpdateInventory(response);
	}
}

// Attempt to put an item into a slot
bool Client::TryToPutInSlot(int16_t slot, int16_t &id, int8_t &amount, int16_t &damage) {
    // First, try to stack into existing slots
    if (player->inventory[slot].id == id && player->inventory[slot].damage == damage) {
        // Skip the slot if its full
        if (player->inventory[slot].amount >= MAX_STACK) {
            return false;
        }
        // If we fill the slot, we're done
        if (player->inventory[slot].amount + amount <= MAX_STACK) {
            player->inventory[slot].amount += amount;
            return true;
        }
        // If we fill it but items remain, keep going
        amount -= (MAX_STACK - player->inventory[slot].amount);
        player->inventory[slot].amount = MAX_STACK;
        return false;
    }
    // Secondly, try to stack into empty slots
    if (player->inventory[slot].id == SLOT_EMPTY) {
        player->inventory[slot] = { id, amount, damage };
        return true;
    }
    return false;
}

// Spread the item to any available slots
bool Client::SpreadToSlots(int16_t id, int8_t amount, int16_t damage, int8_t preferredRange) {
    if (preferredRange == 1 || preferredRange == 0) {
        for (int8_t i = INVENTORY_HOTBAR; i <= INVENTORY_HOTBAR_LAST; i++) {
            if (TryToPutInSlot(i, id, amount, damage)) {
                return true;
            }
        }
    }

    if (preferredRange == 2 || preferredRange == 0) {
        for (int8_t i = INVENTORY_ROW_1; i <= INVENTORY_ROW_LAST; i++) {
            if (TryToPutInSlot(i, id, amount, damage)) {
                return true;
            }
        }
    }

    // If there are still items left, inventory is full
    return false;
}

// Get the Players Orientation along a cardinal direction
int8_t Client::GetPlayerOrientation() {
    float limitedYaw = fmod(player->yaw, 360.0f);
    if (limitedYaw < 0) limitedYaw += 360.0f; // Ensure yaw is in [0, 360)

    int roundedYaw = static_cast<int>(round(limitedYaw / 90.0f)) % 4; // Round to nearest multiple of 90

    switch (roundedYaw) {
        case 0: return zPlus;  // 0°   -> +Z
        case 1: return xMinus; // 90°  -> -X
        case 2: return zMinus; // 180° -> -Z
        case 3: return xPlus;  // 270° -> +X
        default: return zPlus; // Should never happen
    }
}

// Give the player the passed item
bool Client::Give(std::vector<uint8_t> &response, int16_t item, int8_t amount, int16_t damage) {
    // Amount is not specified
    if (amount == -1) {
        if (item < BLOCK_MAX) {
            amount = MAX_STACK;
        } else {
            amount = 1;
        }
    }
    // Look for empty slot
    SpreadToSlots(item,amount,damage);
    //inventory[slotId] = Item { item,amount,damage };
    // TODO: This is a horrible solution, please find something better,
    // like checking if the inventory was changed, and only then sending out an UpdateInventory
    UpdateInventory(response);
    return true;
}

// Update the clients shown inventory
bool Client::UpdateInventory(std::vector<uint8_t> &response) {
    std::vector<Item> v(std::begin(player->inventory), std::end(player->inventory));
    Respond::WindowItems(response, 0, v);
    return true;
}

// Change the players held item for all other clients
void Client::ChangeHeldItem(std::vector<uint8_t> &response, int16_t slotId) {
	currentHotbarSlot = (int8_t)slotId;
    Item i = GetHeldItem();
    Respond::EntityEquipment(response, player->entityId, EQUIPMENT_SLOT_HELD, i.id, i.damage);
}

// Get the hotbar slot the client currently has selected
int16_t Client::GetHotbarSlot() {
    return INVENTORY_HOTBAR + currentHotbarSlot;
}

// Get the currently held item of the client
Item Client::GetHeldItem() {
    return player->inventory[GetHotbarSlot()];
}

// TODO: Implement Right-clicking
// Handle the player clicking a slot in a window
void Client::ClickedSlot(
	[[maybe_unused]] std::vector<uint8_t> &response,
	[[maybe_unused]] int8_t windowId,
	int16_t slotId,
	[[maybe_unused]] bool rightClick,
	[[maybe_unused]] int16_t actionNumber,
	bool shift,
	[[maybe_unused]] int16_t id,
	[[maybe_unused]] int8_t amount,
	[[maybe_unused]] int16_t damage)
{
	// We're interacting with a window we're not
	// currently looking at; something messed up!
	if (windowId != windowIndex) {
		std::cerr << "Was looking at " << int(windowIndex) << " but got click for " << int(windowId) << "!" << std::endl;
		return;
	}

	switch(activeWindow) {
		case INVENTORY_DISPENSER:
			std::cout << "Interacted with Dispenser Inventory" << std::endl;
			break;
		case INVENTORY_FURNACE:
			std::cout << "Interacted with Furnace Inventory" << std::endl;
			break;
		case INVENTORY_WORKBENCH:
			std::cout << "Interacted with Crafting Table Inventory" << std::endl;
			break;
		case INVENTORY_CHEST:
			std::cout << "Interacted with Chest Inventory" << std::endl;
			break;
		// Player Inventory
		default:
			// Shift Click Behavior
			if (shift) {
				// Get item
				Item temp = player->inventory[slotId];
				// Empty slot
				player->inventory[slotId] = {-1,0,0};
				if (slotId >= INVENTORY_HOTBAR) {
					SpreadToSlots(temp.id,temp.amount,temp.damage,2);
				} else {
					SpreadToSlots(temp.id,temp.amount,temp.damage,1);
				}
			}
			
			// If we've clicked outside, throw the items to the ground and clear the slot.
			if (slotId == CLICK_OUTSIDE) {
				hoveringItem = Item {-1,0,0};
				break;
			}

			// If something is being held
			if (hoveringItem.id < BLOCK_STONE) {
				Item temp = hoveringItem;
				hoveringItem = player->inventory[slotId];
				player->inventory[slotId] = temp;
			} else {
				Item temp = player->inventory[slotId];
				player->inventory[slotId] = hoveringItem;
				hoveringItem = temp;
			}
			lastClickedSlot = slotId;
			break;
	}
}

// Clear the clients inventory
void Client::ClearInventory() {
    // Fill inventory with empty slots
    for (int i = 0; i < INVENTORY_MAX_SLOTS; ++i) {
        player->inventory[i] = Item{-1, 0, 0};
    }
}

// Check if the currently held item can be decremented
bool Client::CanDecrementHotbar() {
    Item i = GetHeldItem();
    if (i.id > BLOCK_AIR && i.amount > 0) {
        return true;
    }
    return false;
}

// Decrement the held item by 1
void Client::DecrementHotbar(std::vector<uint8_t> &response) {
    Item* i = &player->inventory[GetHotbarSlot()];
	// TODO: This is bad. Investigate making items better.
	if (IsHoe(i->id)) {
		// Damage Tool
		i->damage -= 1;
	} else {
		// Place item
		i->amount--;
		if (i->amount <= 0) {
			i->id = SLOT_EMPTY;
			i->amount = 0;
			i->damage = 0;
		}
	}
	Respond::SetSlot(response, 0, GetHotbarSlot(), i->id, i->amount, i->damage);
}

// Check if the passed chunk position is visible to the client
bool Client::ChunkIsVisible(Int3 pos) {
	return std::find(visibleChunks.begin(), visibleChunks.end(), pos) != visibleChunks.end();
}

void Client::OpenWindow(int8_t type) {
    windowIndex++;
    switch(type) {
        case INVENTORY_CHEST:
            Respond::OpenWindow(response, windowIndex, INVENTORY_CHEST, "Chest", INVENTORY_CHEST_SIZE);
            break;
        case INVENTORY_CHEST_LARGE:
            Respond::OpenWindow(response, windowIndex, INVENTORY_CHEST, "Large chest", INVENTORY_CHEST_LARGE_SIZE);
            break;
        case INVENTORY_WORKBENCH:
            Respond::OpenWindow(response, windowIndex, INVENTORY_WORKBENCH, "", INVENTORY_WORKBENCH_SIZE);
            break;
        case INVENTORY_FURNACE:
            Respond::OpenWindow(response, windowIndex, INVENTORY_FURNACE, "", INVENTORY_FURNACE_SIZE);
            break;
        case INVENTORY_DISPENSER:
            Respond::OpenWindow(response, windowIndex, INVENTORY_DISPENSER, "", INVENTORY_DISPENSER_SIZE);
            break;
        default:
            windowIndex--;
            return;
    }
    activeWindow = type;
    return;
}

void Client::CloseLatestWindow() {
    Respond::CloseWindow(response, windowIndex);
    windowIndex--;
    return;
}