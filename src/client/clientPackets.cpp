#include "client.h"

#include "server.h"

// --- Packet answers ---
// Respond to any KeepAlive packets
bool Client::HandleKeepAlive() {
	Respond::KeepAlive(response);
	return true;
}

// Engage with the handshake packet
bool Client::HandleHandshake() {
	username = EntryToString16(message, offset);
	auto &server = Betrock::Server::Instance();
	if (server.IsWhitelistEnabled() && !server.IsWhitelist(username)) {
		DisconnectClient("Not on whitelist!");
		return false;
	}
	// If we find a client with the username already connected,
	// don't let them in, as this causes rendering bugs
	Client* oc = server.FindClientByUsername(username);
	if (oc != this) {
		DisconnectClient("Client already on server");
		return false;
	}
	Respond::Handshake(response);
	SetConnectionStatus(ConnectionStatus::LoggingIn);
	return true;
}

// Log the player in and perform checks to ensure a valid connection
bool Client::HandleLoginRequest(World* world) {
	bool firstJoin = true;
	auto &server = Betrock::Server::Instance();
	if (GetConnectionStatus() != ConnectionStatus::LoggingIn) {
		DisconnectClient("Expected Login.");
		return false;
	}

	// Login response
	int protocolVersion = EntryToInteger(message,offset);

	if (protocolVersion != PROTOCOL_VERSION) {
		// If client has wrong protocol, close
		DisconnectClient("Wrong Protocol Version!");
		return false;
	}

	// Player is now certifiably valid, so we create him
	CreatePlayer();

	// TODO: Check if this matches the handshake username
	player->username = EntryToString16(message,offset);
	world = Betrock::Server::Instance().GetWorld(player->dimension);
	if (player->username != username) {
		DisconnectClient("Non-matching username!");
		return false;
	}
	
	EntryToLong(message,offset); // Get map seed
	EntryToByte(message,offset); // Get dimension

	// Fill the players inventory
	if (player->Load()) {
		firstJoin = false;
	}

	// Accept the Login
	Respond::Login(response,player->entityId,world->seed,0);
	Betrock::Logger::Instance().Info(player->username + " logged in with entity id " + std::to_string(player->entityId) + " at " + player->position.str());
	Respond::ChatMessage(broadcastResponse, "§e" + player->username + " joined the game.");

  	const auto &spawnPoint = server.GetSpawnPoint();

	// Set the Respawn Point, Time and Player Health
	Respond::SpawnPoint(response,spawnPoint);
	Respond::Time(response,server.GetServerTime());
	// This is usually only done if the players health isn't full upon joining
	if (player->health != HEALTH_MAX) {
		Respond::UpdateHealth(response,player->health);
	}

	if (firstJoin) {
		// Place the player at spawn
		// TODO: Search for a block for the player to spawn on
		player->position = Int3ToVec3(spawnPoint);

		// Give starter items
		Give(response,ITEM_PICKAXE_DIAMOND);
		Give(response,ITEM_AXE_DIAMOND);
		Give(response,ITEM_SHOVEL_DIAMOND);
		Give(response,BLOCK_STONE);
		Give(response,BLOCK_COBBLESTONE);
		Give(response,BLOCK_PLANKS);
	} else {
		UpdateInventory(response);
	}

	SendPlayerEntity(broadcastOthersResponse, this, player.get());

	// Spawn the other players for the new client
    for (auto other : Betrock::Server::Instance().GetConnectedClients()) {
		if (other.get() == this) { continue; }
		auto otherPlayer = other->GetPlayer();

		SendPlayerEntity(response, other.get(), otherPlayer);
    }
	Respond::ChatMessage(response, std::string("This Server runs on ") + std::string(PROJECT_NAME_VERSION_FULL));
	SendResponse(true);

	// Note: Teleporting automatically loads surrounding chunks,
	// so no further loading is necessary
	player->position.y += 1.0;
	Teleport(response,player->position, player->yaw, player->pitch);
	// ONLY SET THIS AFTER LOGIN HAS FINISHED
	SetConnectionStatus(ConnectionStatus::Connected);
	return true;
}

// Accept any chat messages that're sent.
// If a message starts with a '/', handle it as a command
bool Client::HandleChatMessage() {
	std::string chatMessage = EntryToString16(message, offset);
	if (chatMessage.size() > 0 && chatMessage[0] == '/') {
		std::string command = chatMessage.substr(1);
		CommandManager::Parse(command, this);
	} else {
		std::string sentChatMessage = "<" + player->username + "> " + chatMessage;
		Betrock::Logger::Instance().Info(sentChatMessage);
		Respond::ChatMessage(broadcastResponse,sentChatMessage);
	}
	return true;
}

// Handle any interactions between entities
bool Client::HandleUseEntity() {
	//int32_t originEntityId = EntryToInteger(message, offset);
	EntryToInteger(message, offset);
	//int32_t recipientEntityId = EntryToInteger(message, offset);
	EntryToInteger(message, offset);
	//bool leftClick = EntryToByte(message, offset);
	EntryToByte(message, offset);
	return true;
}

// Handle the player pressing the respawn button
bool Client::HandleRespawn() {
	//int8_t dimension = EntryToByte(message, offset);
	EntryToByte(message, offset);
	Respawn(response);
	return true;
}

// Handle when the Client claims to be standing on solid ground
bool Client::HandlePlayerGrounded() {
	player->onGround = EntryToByte(message, offset);
	return true;
}

// Handle Player Position packets and relay this information to other clients
bool Client::HandlePlayerPosition() {
	Vec3 newPosition;
	double newStance;
	newPosition.x = EntryToDouble(message,offset);
	newPosition.y = EntryToDouble(message,offset);
	newStance = EntryToDouble(message,offset);
	newPosition.z = EntryToDouble(message,offset);
	player->onGround = EntryToByte(message, offset);
	// If an invalid position was hit, tp the player back
	if (!CheckPosition(newPosition,newStance)) {
		TeleportKeepView(response,player->position);
	}
	UpdatePositionForOthers(false);

	if (CheckIfNewChunksRequired()) {
		DetermineVisibleChunks();
	}
	return true;
}

// Handle Player Look packets and relay this information to other clients
bool Client::HandlePlayerLook() {
	player->yaw = EntryToFloat(message,offset);
	player->pitch = EntryToFloat(message,offset);
	player->onGround = EntryToByte(message, offset);
	Respond::EntityLook(broadcastOthersResponse,player->entityId, ConvertFloatToPackedByte(player->yaw), ConvertFloatToPackedByte(player->pitch));
	return true;
}

// Handle Player Position and Look packets and relay this information to other clients
bool Client::HandlePlayerPositionLook() {
	Vec3 newPosition;
	double newStance;
	newPosition.x = EntryToDouble(message,offset);
	newPosition.y = EntryToDouble(message,offset);
	newStance 	  = EntryToDouble(message,offset);
	newPosition.z = EntryToDouble(message,offset);
	player->yaw   = EntryToFloat(message,offset);
	player->pitch = EntryToFloat(message,offset);
	player->onGround = EntryToByte(message, offset);
	CheckPosition(newPosition,newStance);

	UpdatePositionForOthers(true);

	if (CheckIfNewChunksRequired()) {
		DetermineVisibleChunks();
	}
	return true;
}

// Handle the client changing their held item
bool Client::HandleHoldingChange() {
	int16_t slot = EntryToShort(message, offset);
	ChangeHeldItem(broadcastOthersResponse,slot);
	return true;
}

// Handle the client sending out an animation
bool Client::HandleAnimation() {
	int32_t entityId = EntryToInteger(message, offset);
	int8_t animation = EntryToByte(message, offset);
	// Only send this to other clients
	Respond::Animation(broadcastOthersResponse, entityId, animation);
	return true;
}

// Handle a client performing an Entity Action
bool Client::HandleEntityAction() {
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

// Handle the client digging/destroying a block
bool Client::HandlePlayerDigging(World* world) {
	int8_t status = EntryToByte(message, offset);
	int32_t x = EntryToInteger(message, offset);
	int8_t y = EntryToByte(message, offset);
	int32_t z = EntryToInteger(message, offset);
	//int8_t face = EntryToByte(message, offset);
	EntryToByte(message, offset);

	Int3 pos = Int3(x,y,z);
	int8_t blockType = world->GetBlockType(pos);
	int8_t blockMeta = world->GetBlockMeta(pos);
	//Block* targetedBlock = world->GetBlock(pos);
	
	if (debugPunchBlockInfo) {
		//Betrock::Logger::Instance().Debug(IdToLabel((int)targetedBlock->type) + " " + targetedBlock->str() + " at " + pos.str());
	}

	// If the block is broken or instantly breakable
	if (status == 2 || player->creativeMode || IsInstantlyBreakable(blockType)) {
		Respond::Soundeffect(broadcastOthersResponse,BLOCK_BREAK,pos,blockType);
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
			Item item = Item{blockType,1,blockMeta};
			if (!player->creativeMode) {
				item = GetDrop(item);
			}
			Give(response,item.id,item.amount,item.damage);
		}
		// Special handling for multi-block blocks
		if (blockType == BLOCK_DOOR_WOOD ||
			blockType == BLOCK_DOOR_IRON)
		{
			Int3 nPos = pos;
			if (blockMeta & 0b1000) {
				// Interacted with Top
				// Update Bottom
				nPos = pos + Int3{0,-1,0};
			} else {
				// Interacted with Bottom
				// Update Top
				nPos = pos + Int3{0,1,0};
			}
			if (world->GetBlockType(nPos)==blockType) {
				world->PlaceBlockUpdate(nPos, BLOCK_AIR, 0);
			}
		}
		// Only get rid of the block here to avoid unreferenced pointers
		world->PlaceBlockUpdate(pos, BLOCK_AIR, 0);
	}

	// Check if the targeted block is interactable
	if (IsInteractable(blockType)) {
		world->InteractWithBlock(pos);
		return true;
	}
	return true;
}

// Handle the client attempting to place a block
bool Client::HandlePlayerBlockPlacement(World* world) {
	int32_t x = EntryToInteger(message, offset);
	int8_t y = EntryToByte(message, offset);
	int32_t z = EntryToInteger(message, offset);
	int8_t face = EntryToByte(message, offset);
	// All of the following is pretty optional,
	// Since we get the players' desired block from
	// The Server-side inventory anyways
	int16_t id = EntryToShort(message, offset);
	
	//int8_t amount = 0;
	//int16_t damage = 0;
	if (id >= 0) {
		EntryToByte(message, offset);
		EntryToShort(message, offset);
		//amount = EntryToByte(message, offset);
		//damage = EntryToShort(message, offset);
	}

	Int3 pos = Int3{x,y,z};
	int8_t blockType = world->GetBlockType(pos);
	int8_t blockMeta = world->GetBlockMeta(pos);
	if (blockType == BLOCK_AIR) { return false; }

	// Check if the targeted block is interactable
	if (IsInteractable(blockType)) {
		if (!HasInventory(blockType)) {
			world->InteractWithBlock(pos);
			return true;
		}
		switch(blockType) {
			case BLOCK_CRAFTING_TABLE:
				OpenWindow(INVENTORY_CRAFTING_TABLE);
				break;
			// TODO: Figure out how to handle large chests
			// Possibly by checking for surrounding chests,
			// then adding up their capacities?
			// Gotta recreate the 3-size chest bug somehow!
			case BLOCK_CHEST:
				OpenWindow(INVENTORY_CHEST);
				SendResponse(true);
				UpdateInventory(response, pos);
				break;
			case BLOCK_FURNACE:
			case BLOCK_FURNACE_LIT:
				OpenWindow(INVENTORY_FURNACE);
				break;
			case BLOCK_DISPENSER:
				OpenWindow(INVENTORY_DISPENSER);
				break;
		}
		return true;
	}

	// This packet has a special case where X, Y, Z, and Direction are all -1.
	// This special packet indicates that the currently held item for the player should have
	// its state updated such as eating food, shooting bows, using buckets, etc.

	// Apparently this also handles the player standing inside the block its trying to place in
	// This is apparently for the player trying to *use* the item(?)
	if (x == -1 && y == -1 && z == -1 && face == -1) {
		return false;
	}

	// Place a block if we can
	if (!CanDecrementHotbar()) return false;

	// Check if the server-side inventory item is valid
	Item i = player->inventory[INVENTORY_HOTBAR+currentHotbarSlot];
	
	// Special handling for Slabs
	if (
		blockType == BLOCK_SLAB &&
		blockMeta == i.damage &&
		face == yPlus
	) {
		world->PlaceBlockUpdate(pos,BLOCK_DOUBLE_SLAB,i.damage);
	} else {
		// Get the block we need to place
		BlockToFace(pos,face);
		Block b = GetPlacedBlock(world,pos,face,player->yaw,GetPlayerOrientation(),i.id,i.damage);
		if (!IsValidPlacement(b.type, pos)) return false;
		if (b.type == SLOT_EMPTY) return false;
		switch(b.type) {
			case BLOCK_SPONGE:
				world->PlaceSponge(pos);
				break;
			case BLOCK_CHEST:
				world->AddTileEntity(std::make_unique<ChestTile>(pos));
				world->PlaceBlockUpdate(pos,b.type,b.meta);
				break;
			default:
				world->PlaceBlockUpdate(pos,b.type,b.meta);
				break;
		}
	}
	// Immediately give back the item if we're in creative mode
	if (player->creativeMode) {
		Item i = GetHeldItem();
		Respond::SetSlot(response,0,GetHotbarSlot()+9,i.id,i.amount,i.damage);
	} else {
		DecrementHotbar(response);
	}
	return true;
}

// Handle the Client closing a Window
bool Client::HandleCloseWindow() {
	int8_t windowId = EntryToByte(message, offset);
	//std::cout << int(windowId) << std::endl;
	// Ignore inventory
	if (windowId == 0) return true;
	
	if (windowId == windowIndex) {
		windowIndex--;
	} else {
		CloseLatestWindow();
	}

	// Ensure we can never go lower than the default inventory
	if (windowIndex <= 0) {
		windowIndex = 0;
		activeWindowType = INVENTORY_NONE;
	}
	return true;
}

// Handle the Client clicking while a Window is open
bool Client::HandleWindowClick() {
	int8_t window 		= EntryToByte(message, offset);
	int16_t slot 		= EntryToShort(message,offset);
	[[maybe_unused]] int8_t rightClick 	= EntryToByte(message, offset);
	[[maybe_unused]] int16_t actionNumber= EntryToShort(message,offset);
	[[maybe_unused]] int8_t shift 		= EntryToByte(message, offset);
	[[maybe_unused]] int16_t itemId		= EntryToShort(message,offset);
	[[maybe_unused]] int8_t itemCount	= 1;
	[[maybe_unused]] int16_t itemUses	= 0;
	//std::cout << int(window) << " (" << int(slot) << ")" << std::endl;
	if (itemId > 0) {
		itemCount		= EntryToByte(message, offset);
		itemUses		= EntryToShort(message,offset);
	}
	ClickedSlot(response, window,slot,(bool)rightClick,actionNumber,shift,itemId,itemCount,itemUses);
	return true;
}

bool Client::HandleUpdateSign() {
	int32_t x 		  = EntryToInteger(message, offset);
	int16_t y 		  = EntryToShort(message,offset);
	int32_t z 		  = EntryToInteger(message, offset);
	std::array<std::string, 4> lines;
	lines[0] = EntryToString16(message,offset);
	lines[1] = EntryToString16(message,offset);
	lines[2] = EntryToString16(message,offset);
	lines[3] = EntryToString16(message,offset);

	auto wm = Betrock::Server::Instance().GetWorldManager(player->dimension);
	wm->world.AddTileEntity(std::make_unique<SignTile>(Int3{x, y, z}, lines));
	Respond::UpdateSign(broadcastResponse,Int3{x,y,z},lines);
	return true;
}

// Handle the Client attempting to disconnect
bool Client::HandleDisconnect() {
	std::string disconnectMessage = EntryToString16(message, offset);
	DisconnectClient(disconnectMessage,true);
	return true;
}

// This is incredibly ugly, but it makes the Server show up in 1.6+ Server Lists!
// Handle the 1.6+ Server List Packet
void Client::HandleLegacyPing() {
	response.push_back((uint8_t)Packet::Disconnect);
	int32_t maximumPlayers = Betrock::Server::Instance().GetMaximumPlayers();
	std::vector<std::string> strings = {
		std::to_string(PROTOCOL_VERSION),
		"b1.7.3",
		Betrock::Server::Instance().GetMotd(),
		std::to_string(Betrock::Server::Instance().GetConnectedClients().size()),
		std::to_string(maximumPlayers == -1 ? 0 : maximumPlayers)
	};

	// Acounts for $1 already being there
	size_t combinedSize = 3;
	for (size_t i = 0; i < strings.size(); i++) {
		combinedSize += strings[i].size();
		if (i < strings.size() - 1) {
			// Account for null characters
			combinedSize++;
		}
	}

	// Length of string
	response.push_back((combinedSize >> 8) & 0xFF);
	response.push_back((combinedSize & 0xFF));

	// $1 start
	response.push_back(0x00);
	response.push_back(0xA7);
	response.push_back(0x00);
	response.push_back('1');
	response.push_back(0x00);
	response.push_back(0x00);
	
	// Remaining data
	for (size_t i = 0; i < strings.size(); i++) {
		for (auto c : strings[i]) {
			response.push_back(0x00);
			response.push_back(c);
		}
		if (i < strings.size() - 1) {
			// Account for null characters
			response.push_back(0x00);
			response.push_back(0x00);
		}
	}

	SendResponse(true);
	SetConnectionStatus(ConnectionStatus::Disconnected);
}