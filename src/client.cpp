#include "client.h"

#include "server.h"

#include <ranges>

Client::Client(Player* player) {
	this->player = player;	
}

bool Client::CheckPosition(Player* player, Vec3 &newPosition, double &newStance) {
	player->previousPosition = player->position;

	player->position = newPosition;
	player->stance = newStance + 1.62;
	return true;
}

ssize_t Client::Setup() {
	// Set stuff up for the next batch of packets
	response.clear();
	broadcastResponse.clear();
	broadcastOthersResponse.clear();
	offset = 0;
	previousOffset = 0;

	// Read Data
	return read(player->client_fd, message, PACKET_MAX);
}

void Client::PrintReceived(Packet packetType, ssize_t bytes_received) {
	if (debugReceivedPacketType) {
		std::cout << "Received " << PacketIdToLabel(packetType) << " from " << player->username << "! (" << bytes_received << " Bytes)" << std::endl;
	}
	if (debugReceivedBytes) {
		for (uint i = 0; i < bytes_received; i++) {
			std::cout << std::hex << (int)message[i];
			if (i < bytes_received-1) {
				std::cout << ", ";
			}
		}
		std::cout << std::dec << std::endl;
	}
}

void Client::PrintRead(Packet packetType) {
	std::cout << "Read " << PacketIdToLabel(packetType) << " from " << player->username << "! (" << offset-previousOffset << " Bytes)" << std::endl;
	for (uint i = previousOffset; i < offset; i++) {
		std::cout << std::hex << (int)message[i];
		if (i < offset-1) {
			std::cout << ", ";
		}
	}
	std::cout << std::dec << std::endl;
	previousOffset = offset;
}

bool CheckIfNewChunksRequired(Player* player) {
	Vec3 lastPos = player->lastChunkUpdatePosition;
	Vec3 newPos = player->position;
	// Remove vertical component
	lastPos.y = 0;
	newPos.y = 0;
	if (GetDistance(lastPos,newPos) > 16) {
		return true;
	}
	return false;
}

void ProcessChunk(std::vector<uint8_t>& response, const Int3& position, WorldManager* wm, Player* player) {
	// TODO: This is awful to do for every chunk :(
    // Skip processing if chunk is already visible
    if (std::find(player->visibleChunks.begin(), player->visibleChunks.end(), position) != player->visibleChunks.end()) {
        return;
    }

    // Get existing chunk data
    auto chunkData = wm->world.GetChunkData(position);
    if (!chunkData) {
        // Queue chunk generation if missing
        wm->AddChunkToQueue(position.x, position.z, player);
        Respond::PreChunk(response, position.x, position.z, 1); // Tell client chunk is being worked on
        return;
    }
    
    player->newChunks.push_back(position);
}

void SendChunksAroundPlayer(std::vector<uint8_t> &response, Player* player, bool forcePlayerAsCenter) {
    auto &server = Betrock::Server::Instance();

    Int3 centerPos;
	if (forcePlayerAsCenter) {
		centerPos = Vec3ToInt3(player->position);
	} else {
    	Vec3 delta = player->position - player->lastChunkUpdatePosition;
		centerPos = Vec3ToInt3(player->position+delta);
	}
    Int3 playerChunkPos = BlockToChunkPosition(centerPos);
    int32_t pX = playerChunkPos.x;
    int32_t pZ = playerChunkPos.z;

    auto chunkDistance = server.GetChunkDistance();

    // Remove chunks that are out of range
    for (auto it = player->visibleChunks.begin(); it != player->visibleChunks.end(); ) {
        int distanceX = abs(pX - it->x);
        int distanceZ = abs(pZ - it->z);
        if (distanceX > chunkDistance || distanceZ > chunkDistance) {
            Respond::PreChunk(response, it->x, it->z, 0); // Tell client chunk is no longer visible
            it = player->visibleChunks.erase(it);
			//std::cout << "Deleted " << it->x << ", " << it->z << std::endl;
        } else {
            ++it;
        }
    }

    auto wm = server.GetWorldManager(player->worldId);

	// Iterate over all chunks within a bounding box defined by chunkDistance
	for (int r = 0; r < chunkDistance; r++) {
		// Top and Bottom rows
		for (int x = -r; x <= r; x++) {
			for (int z : {-r, r}) {
				Int3 position = XyzToInt3(x+pX, 0, z+pZ);
				ProcessChunk(response, position, wm, player);
			}
		}
		// Left and Right columns (excluding corners to avoid duplicates)
		for (int z = -r + 1; z <= r - 1; z++) {
			for (int x : {-r, r}) {
				Int3 position = XyzToInt3(x+pX, 0, z+pZ);
				ProcessChunk(response, position, wm, player);
			}
		}
	}

    player->lastChunkUpdatePosition = player->position;
}

void Client::SendNewChunks() {
	// Send chunks in batches of 5
	int sentThisCycle = 5;
	auto wm = Betrock::Server::Instance().GetWorldManager(player->worldId);
  	std::lock_guard<std::mutex> lock(player->newChunksMutex);
	while(sentThisCycle > 0) {
		if(!player->newChunks.empty()) {
			auto nc = player->newChunks.begin();
			auto chunkData = wm->world.GetChunkData(*nc);
			if (!chunkData) {
				// We'll just drop this chunk
				player->newChunks.erase(nc);
				return;
			}

			// Send chunk to player
			size_t compressedSize = 0;
			auto chunk = CompressChunk(chunkData.get(), compressedSize);

			if (chunk) {
				//std::cout << "Sent " << nc->x << ", " << nc->z << std::endl;
				Respond::PreChunk(response, nc->x, nc->z, 1);
				player->visibleChunks.push_back(Int3{nc->x,0,nc->z});

				Respond::Chunk(
					response, 
					Int3{nc->x<<4,0,nc->z<<4}, 
					CHUNK_WIDTH_X - 1, 
					CHUNK_HEIGHT - 1, 
					CHUNK_WIDTH_Z - 1, 
					compressedSize, 
					chunk.get()
				);
			}
			// Better to remove the entry either way if compression fails,
			// otherwise we may get an infinite build-up of failing chunks
			player->newChunks.erase(nc);
		}
		sentThisCycle--;
	}
}

void Client::Respond(ssize_t bytes_received) {
	SendToPlayer(response, player);
	BroadcastToPlayers(broadcastResponse);
	BroadcastToPlayers(broadcastOthersResponse, player);
	if (debugNumberOfPacketBytes) {
		std::cout << "--- " << offset << "/" << bytes_received << " Bytes Read from Packet ---" << std::endl << std::endl; 
	}
}

void HandlePacket(Client &client) {
	auto serverTime = Betrock::Server::Instance().GetServerTime();
	int64_t lastPacketTime = serverTime;
	// Prep for next packet
	ssize_t bytes_received = client.Setup();

	// If we receive no Data from the player, such as when they 
	if (bytes_received <= 0) {
		perror("read");
		Disconnect(client.player,"No data.");
		return;
	}

	lastPacketTime = serverTime;

	// Read packet bundle until end
	if (debugReceivedBundleDelimiter) {
		std::cout << "--- Start of Packet bundle ---" << std::endl;
	}
	while (client.offset < bytes_received && client.player->connectionStatus > ConnectionStatus::Disconnected) {
		int8_t packetIndex = EntryToByte(client.message,client.offset);
		Packet packetType = (Packet)packetIndex;

		// Provide debug info
		if (debugReceivedBytes || debugReceivedPacketType) {
			client.PrintReceived(packetType,bytes_received);
		}

		// Get the current Dimension
		World* world = Betrock::Server::Instance().GetWorld(client.player->worldId);
		
		// The Client tries to join the Server
		switch(packetType) {
			case Packet::KeepAlive:
				client.KeepAlive();
				break;
			case Packet::LoginRequest:
				client.LoginRequest();
				break;
			case Packet::Handshake:
				client.Handshake();
				break;
			case Packet::ChatMessage:
				client.ChatMessage();
				break;
			case Packet::UseEntity:
				client.UseEntity();
				break;
			case Packet::Respawn:
				client.Respawn();
				break;
			case Packet::Player:
				client.PlayerGrounded();
				break;
			case Packet::PlayerPosition:
				client.PlayerPosition();
				break;
			case Packet::PlayerLook:
				client.PlayerLook();
				break;
			case Packet::PlayerPositionLook:
				client.PlayerPositionLook();
				break;
			case Packet::HoldingChange:
				client.HoldingChange();
				break;
			case Packet::Animation:
				client.Animation();
				break;
			case Packet::EntityAction:
				client.EntityAction();
				break;
			case Packet::PlayerDigging:
				client.PlayerDigging(world);
				break;
			case Packet::PlayerBlockPlacement:
				client.PlayerBlockPlacement(world);
				break;
			case Packet::CloseWindow:
				client.CloseWindow();
				break;
			case Packet::WindowClick:
				client.WindowClick();
				break;
			case Packet::Disconnect:
				client.DisconnectClient();
				break;
			default:
				Betrock::Logger::Instance().Log("Unhandled Server-bound packet: " + std::to_string(packetIndex), LOG_WARNING);
				break;
		}
		if (client.player != nullptr && client.player->connectionStatus == ConnectionStatus::Connected) {
			if (debugPlayerStatus) {
				client.player->PrintStats();
			}
			// TODO: Fix this from killing the player during lag
			// Kill player if he goes below 0,0
			/*
			if (client.player->position.y < 0) {
				Respond::UpdateHealth(client.response,0);
			}
			*/
		}
		if (debugReceivedRead) {
			client.PrintRead(packetType);
		}
	}
	client.SendNewChunks();
	client.Respond(bytes_received);
}

// Give each Player their own thread
void HandleClient(Player* player) {
  auto &server = Betrock::Server::Instance();
	// Assign player
	Client client = Client(player);

	// While the player is connected, read packets from them
	while (player->connectionStatus > ConnectionStatus::Disconnected) {
		HandlePacket(client);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000/TICK_SPEED)); // Sleep for half a second
	}
	std::lock_guard<std::mutex> lock(server.GetConnectedPlayerMutex());
	int clientFdToDisconnect = player->client_fd;
	auto &connectedPlayers = server.GetConnectedPlayers();
    auto it = std::ranges::find(std::ranges::views::all(connectedPlayers), player);
    if (it != connectedPlayers.end()) {
        connectedPlayers.erase(it);
    }

  	// TODO: >:c (translation: smart pointer)
	delete player;
	player = nullptr;

	close(clientFdToDisconnect);
	return;
}

// --- Packet answers ---

bool Client::KeepAlive() {
	Respond::KeepAlive(response);
	return true;
}

bool Client::Handshake() {
	if (player->connectionStatus != ConnectionStatus::Handshake) {
		Disconnect(player,"Expected Handshake.");
		return false;
	}
	player->username = EntryToString16(message, offset);
	Respond::Handshake(response);
	player->connectionStatus = ConnectionStatus::LoggingIn;
	return true;
}

bool Client::LoginRequest() {
  auto &server = Betrock::Server::Instance();
	if (player->connectionStatus != ConnectionStatus::LoggingIn) {
		Disconnect(player,"Expected Login.");
		return false;
	}

	// Login response
	int protocolVersion = EntryToInteger(message,offset);
	std::string username = EntryToString16(message,offset); // Get username again
	if (username != player->username) {
		Disconnect(player,"Client has mismatched username.");
		return false;
	} 
	EntryToLong(message,offset); // Get map seed
	EntryToByte(message,offset); // Get dimension

	if (protocolVersion != PROTOCOL_VERSION) {
		// If client has wrong protocol, close
		Disconnect(player,"Wrong Protocol Version!");
		return false;
	}
	// Accept the Login
	Respond::Login(response,player->entityId,1,0);
	Betrock::Logger::Instance().Message(username + " logged in with entity id " + std::to_string(player->entityId) + " at (" + std::to_string(player->position.x) + ", " + std::to_string(player->position.y) + ", " + std::to_string(player->position.z) + ")");
	Respond::ChatMessage(broadcastResponse, "§e" + username + " joined the game.", 0);

  	const auto &spawnPoint = server.GetSpawnPoint();

	// Set the Respawn Point, Time and Player Health
	Respond::SpawnPoint(response,Vec3ToInt3(spawnPoint));
	Respond::Time(response,server.GetServerTime());
	Respond::UpdateHealth(response,player->health);

	// Fill the players inventory
	player->Give(response,ITEM_PICKAXE_DIAMOND);
	player->Give(response,ITEM_AXE_DIAMOND);
	player->Give(response,ITEM_SHOVEL_DIAMOND);
	player->Give(response,BLOCK_STONE);
	player->Give(response,BLOCK_COBBLESTONE);
	player->Give(response,BLOCK_PLANKS);
	//player->UpdateInventory();

	// Place the player at spawn
	// Note: Teleporting automatically loads surrounding chunks,
	// so no further loading is necessary
	player->Teleport(response,spawnPoint);
	//SendChunksAroundPlayer(response,player);
	//SendNewChunks();

	// Create the player for other players
	Respond::NamedEntitySpawn(
		broadcastOthersResponse,
		player->entityId,
		player->username,
		Vec3ToInt3(player->position),
		player->yaw,
		player->pitch,
		player->inventory[INVENTORY_HOTBAR].id
	);

	// Spawn the other players for the new client
    for (Player* others : Betrock::Server::Instance().GetConnectedPlayers()) {
		if (others == player) { continue; }
		Respond::NamedEntitySpawn(
			response,
			others->entityId,
			others->username,
			Vec3ToInt3(others->position),
			others->yaw,
			others->pitch,
			others->inventory[INVENTORY_HOTBAR + others->currentHotbarSlot].id
		);
		
		// Apparently needed to entities show up where they need to
		Respond::EntityTeleport(
			response,
			others->entityId,
			Vec3ToEntityInt3(others->position),
			ConvertFloatToPackedByte(others->yaw),
			ConvertFloatToPackedByte(others->pitch)
		);

    }
	player->connectionStatus = ConnectionStatus::Connected;
	Respond::ChatMessage(response, std::string("This Server runs on ") + std::string(PROJECT_NAME_VERSION), false);
	return true;
}

bool Client::ChatMessage() {
	std::string chatMessage = EntryToString16(message, offset);
	if (chatMessage.size() > 0 && chatMessage[0] == '/') {
		std::string command = chatMessage.substr(1);
		Command::Parse(command, player);
	} else {
		std::string sentChatMessage = "<" + player->username + "> " + chatMessage;
		Respond::ChatMessage(broadcastResponse,sentChatMessage);
	}
	return true;
}

bool Client::UseEntity() {
	int32_t originEntityId = EntryToInteger(message, offset);
	int32_t recipientEntityId = EntryToInteger(message, offset);
	bool leftClick = EntryToByte(message, offset);
	return true;
}

bool Client::Respawn() {
	int8_t dimension = EntryToByte(message, offset);
	player->Respawn(response);
	return true;
}

bool Client::PlayerGrounded() {
	player->onGround = EntryToByte(message, offset);
	return true;
}

bool Client::PlayerPosition() {
	Vec3 newPosition;
	double newStance;
	newPosition.x = EntryToDouble(message,offset);
	newPosition.y = EntryToDouble(message,offset);
	newStance = EntryToDouble(message,offset);
	newPosition.z = EntryToDouble(message,offset);
	player->onGround = EntryToByte(message, offset);
	CheckPosition(player,newPosition,newStance);

	Respond::EntityTeleport(
		broadcastOthersResponse,
		player->entityId,
		Vec3ToEntityInt3(player->position),
		ConvertFloatToPackedByte(player->yaw),
		ConvertFloatToPackedByte(player->pitch)
	);

	if (CheckIfNewChunksRequired(player)) {
		SendChunksAroundPlayer(response,player);
	}
	return true;
}

bool Client::PlayerLook() {
	player->yaw = EntryToFloat(message,offset);
	player->pitch = EntryToFloat(message,offset);
	player->onGround = EntryToByte(message, offset);
	Respond::EntityLook(broadcastOthersResponse,player->entityId, ConvertFloatToPackedByte(player->yaw), ConvertFloatToPackedByte(player->pitch));
	return true;
}

bool Client::PlayerPositionLook() {
	Vec3 newPosition;
	double newStance;
	newPosition.x = EntryToDouble(message,offset);
	newPosition.y = EntryToDouble(message,offset);
	newStance 	  = EntryToDouble(message,offset);
	newPosition.z = EntryToDouble(message,offset);
	player->yaw   = EntryToFloat(message,offset);
	player->pitch = EntryToFloat(message,offset);
	player->onGround = EntryToByte(message, offset);
	CheckPosition(player,newPosition,newStance);
	if (CheckIfNewChunksRequired(player)) {
		SendChunksAroundPlayer(response,player);
	}
	return true;
}

bool Client::HoldingChange() {
	int16_t slot = EntryToShort(message, offset);
	player->ChangeHeldItem(broadcastOthersResponse,slot);
	return true;
}

bool Client::Animation() {
	int32_t entityId = EntryToInteger(message, offset);
	int8_t animation = EntryToByte(message, offset);
	// Only send this to other clients
	Respond::Animation(broadcastOthersResponse, entityId, animation);
	return true;
}

bool Client::EntityAction() {
	int32_t entityId = EntryToInteger(message, offset);
	int8_t action = EntryToByte(message, offset);
	// some EntityMetadata info
	// A BITMASK
	// - Bit 0 is for Crouching
	// - Bit 1 is for On Fire
	// - Bit 2 is for Sitting
	// All other bits are irrelevant, it looks like
	switch(action) {
		case 1:
			player->crouching = true;
			break;
		case 2:
			player->crouching = false;
			break;
		default:
			break;
	}
	int8_t responseByte = (player->sitting << 2 | player->crouching << 1 | player->onFire);
	Respond::EntityMetadata(broadcastOthersResponse, entityId, responseByte);
	return true;
}

bool Client::PlayerDigging(World* world) {
	int8_t status = EntryToByte(message, offset);
	int32_t x = EntryToInteger(message, offset);
	int8_t y = EntryToByte(message, offset);
	int32_t z = EntryToInteger(message, offset);
	int8_t face = EntryToByte(message, offset);

	Int3 pos = XyzToInt3(x,y,z);
	if (status == 2 || player->creativeMode) {
		Respond::BlockChange(broadcastResponse,pos,0,0);
		Block b = world->BreakBlock(pos);
		Respond::Soundeffect(broadcastOthersResponse,BLOCK_BREAK,pos,b.type);
		if (doTileDrops && !player->creativeMode) {
			// TODO: This works now,
			// but results in entities piling up
			// We need server-side managed entities
			/*
			Respond::PickupSpawn(
				broadcastResponse,
				Betrock::Server::Instance().GetLatestEntityId(),
				b.type,
				1,
				b.meta,
				Int3ToEntityInt3(pos),
				0,0,0
			);
			*/
			player->Give(response,b.type,1,b.meta);
		}
	}
	return true;
}

bool Client::BlockTooCloseToPosition(Int3 position) {
    // Player's bounding box
    double playerMinX = player->position.x - 0.3f;
    double playerMaxX = player->position.x + 0.3f;

    double playerMinY = player->position.y;
	double playerMaxY = player->position.y + 1.8f;
	if (player->crouching) {
		playerMaxY = player->position.y + 1.5f;  // Correct: modifies the existing variable
	}

    double playerMinZ = player->position.z - 0.25f;
    double playerMaxZ = player->position.z + 0.25f;

    // Block's bounding box (aligned to integer grid)
    double blockMinX = static_cast<double>(position.x);
    double blockMaxX = blockMinX + 1.0f;

    double blockMinY = static_cast<double>(position.y);
    double blockMaxY = blockMinY + 1.0f;

    double blockMinZ = static_cast<double>(position.z);
    double blockMaxZ = blockMinZ + 1.0f;

    // Check for overlap on all three axes
    bool overlapX = playerMinX < blockMaxX && playerMaxX > blockMinX;
    bool overlapY = playerMinY < blockMaxY && playerMaxY > blockMinY;
    bool overlapZ = playerMinZ < blockMaxZ && playerMaxZ > blockMinZ;

    // If all axes overlap, the bounding boxes intersect
    return overlapX && overlapY && overlapZ;
}

bool Client::PlayerBlockPlacement(World* world) {
	int32_t x = EntryToInteger(message, offset);
	int8_t y = EntryToByte(message, offset);
	int32_t z = EntryToInteger(message, offset);
	int8_t direction = EntryToByte(message, offset);
	int16_t id = EntryToShort(message, offset);
	int8_t amount = 0;
	int16_t damage = 0;
	if (id >= 0) {
		amount = EntryToByte(message, offset);
		damage = EntryToShort(message, offset);
	}

	BlockToFace(x,y,z,direction);
	Int3 pos = XyzToInt3(x,y,z);
	// This packet has a special case where X, Y, Z, and Direction are all -1.
	// This special packet indicates that the currently held item for the player should have
	// its state updated such as eating food, shooting bows, using buckets, etc.

	// Apparently this also handles the player standing inside the block its trying to place in
	if (x == -1 && y == -1 && z == -1 && direction == -1) {
		return true;
	}
	// Place a block if we can
	if (id > BLOCK_AIR && id < BLOCK_MAX && !BlockTooCloseToPosition(pos) && player->CanDecrementHotbar()) {
		//std::cout << BlockTooCloseToPosition(pos) << ": " << pos << " - " << player->position << std::endl;
		Item i = player->inventory[INVENTORY_HOTBAR+player->currentHotbarSlot];
		Respond::BlockChange(broadcastResponse,pos,(int8_t)i.id,(int8_t)i.damage);
		world->PlaceBlock(pos,(int8_t)i.id,(int8_t)i.damage);
		// Immediately give back item if we're in creative mode
		if (player->creativeMode) {
			Item i = player->GetHeldItem();
			id = i.id;
			amount = i.amount;
			damage = i.damage;
		} else {
			player->DecrementHotbar(response);
		}
	}
	return true;
}

bool Client::CloseWindow() {
	int8_t window 	= EntryToByte(message, offset);
	activeWindow = INVENTORY_NONE;
	return true;
}

bool Client::WindowClick() {
	int8_t window 		= EntryToByte(message, offset);
	int16_t slot 		= EntryToShort(message,offset);
	int8_t rightClick 	= EntryToByte(message, offset);
	int16_t actionNumber= EntryToShort(message,offset);
	int8_t shift 		= EntryToByte(message, offset);
	int16_t itemId		= EntryToShort(message,offset);
	int8_t itemCount	= 1;
	int16_t itemUses	= 0;
	if (itemId > 0) {
		itemCount		= EntryToByte(message, offset);
		itemUses		= EntryToShort(message,offset);
	}
	player->ClickedSlot(response,window,slot,(bool)rightClick,actionNumber,shift,itemId,itemCount,itemUses);
	return true;
}

// TODO: This completely ignores the disconnect message sent by the player
bool Client::DisconnectClient() {
	std::string disconnectMessage = EntryToString16(message, offset);
	Respond::DestroyEntity(broadcastOthersResponse,player->entityId);
	Disconnect(player,disconnectMessage);
	Respond::ChatMessage(broadcastResponse, "§e" + player->username + " left the game.");
	return true;
}