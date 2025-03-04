#include "client.h"

#include "server.h"

#include <ranges>

// Check if the position of a player is valid
bool Client::CheckPosition(Vec3 &newPosition, double &newStance) {
	player->previousPosition = player->position;

	player->position = newPosition;
	player->stance = newStance + 1.62;
	return true;
}

// Set the client up to receive another packet
ssize_t Client::Setup() {
	// Set stuff up for the next batch of packets
	response.clear();
	broadcastResponse.clear();
	broadcastOthersResponse.clear();
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

// Print the remaining, to-be-read data
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

// Check if the player has moved far enough to warrant sending new chunk data
bool Client::CheckIfNewChunksRequired() {
	Vec3 lastPos = lastChunkUpdatePosition;
	Vec3 newPos = player->position;
	// Remove vertical component
	lastPos.y = 0;
	newPos.y = 0;
	if (GetDistance(lastPos,newPos) > 16) {
		return true;
	}
	return false;
}

// Check if a chunk already exists in memory or if it needs to be loaded from memory first
void Client::ProcessChunk(const Int3& position, WorldManager* wm) {
	// TODO: This is awful to do for every chunk :(
    // Skip processing if chunk is already visible
    if (std::find(visibleChunks.begin(), visibleChunks.end(), position) != visibleChunks.end()) {
        return;
    }

    // Check if the chunk has already been loaded
    if (!wm->world.ChunkExists(position.x,position.z)) {
		// Otherwise queue chunk loading or generation
		wm->AddChunkToQueue(position.x, position.z, this);
		//Respond::PreChunk(response, position.x, position.z, 1); // Tell client chunk is being worked on
		SendResponse(true);
        return;
    }
    // If the chunk is already available, send it over
    newChunks.push_back(position);
}

// Figure out what chunks the player can see and
// add them to the NewChunks queue if any new ones are added
void Client::DetermineVisibleChunks(bool forcePlayerAsCenter) {
    auto &server = Betrock::Server::Instance();

    Int3 centerPos;
	if (forcePlayerAsCenter) {
		centerPos = Vec3ToInt3(player->position);
	} else {
    	Vec3 delta = player->position - lastChunkUpdatePosition;
		centerPos = Vec3ToInt3(player->position+delta);
	}
    Int3 playerChunkPos = BlockToChunkPosition(centerPos);
    int32_t pX = playerChunkPos.x;
    int32_t pZ = playerChunkPos.z;

    auto chunkDistance = server.GetChunkDistance();

    // Remove chunks that are out of range
    for (auto it = visibleChunks.begin(); it != visibleChunks.end(); ) {
        int distanceX = abs(pX - it->x);
        int distanceZ = abs(pZ - it->z);
        if (distanceX > chunkDistance || distanceZ > chunkDistance) {
            Respond::PreChunk(response, it->x, it->z, 0); // Tell client chunk is no longer visible
            it = visibleChunks.erase(it);
			//std::cout << "Deleted " << it->x << ", " << it->z << std::endl;
        } else {
            ++it;
        }
    }

    auto wm = server.GetWorldManager(player->dimension);

	// Iterate over all chunks within a bounding box defined by chunkDistance
	for (int r = 0; r < chunkDistance; r++) {
		// Top and Bottom rows
		for (int x = -r; x <= r; x++) {
			for (int z : {-r, r}) {
				Int3 position = XyzToInt3(x+pX, 0, z+pZ);
				ProcessChunk(position, wm);
			}
		}
		// Left and Right columns (excluding corners to avoid duplicates)
		for (int z = -r + 1; z <= r - 1; z++) {
			for (int x : {-r, r}) {
				Int3 position = XyzToInt3(x+pX, 0, z+pZ);
				ProcessChunk(position, wm);
			}
		}
	}

    lastChunkUpdatePosition = player->position;
}

// Send the chunks from the newChunks queue to the player
void Client::SendNewChunks() {
	// Send chunks in batches of 5
	int sentThisCycle = 5;
	auto wm = Betrock::Server::Instance().GetWorldManager(player->dimension);
	std::lock_guard<std::mutex> lock(newChunksMutex);
	while(sentThisCycle > 0) {
		if(!newChunks.empty()) {
			auto nc = newChunks.begin();
			auto chunkData = wm->world.GetChunkData(*nc);
			if (!chunkData) {
				// We'll just drop this chunk
				newChunks.erase(nc);
				return;
			}

			// Send chunk to player
			size_t compressedSize = 0;
			auto chunk = CompressChunk(chunkData.get(), compressedSize);

			if (chunk) {
				//std::cout << "Sent " << nc->x << ", " << nc->z << std::endl;
				Respond::PreChunk(response, nc->x, nc->z, 1);
				visibleChunks.push_back(Int3{nc->x,0,nc->z});

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
			newChunks.erase(nc);
		}
		sentThisCycle--;
	}
}

// This is incredibly ugly, but it makes the Server show up in 1.6+ Server Lists!
// Handle the 1.6+ Server List Packet
void Client::HandleLegacyPing() {
	response.push_back((uint8_t)Packet::Disconnect);
	response.push_back(0x00);
	response.push_back(0x23);
	response.push_back(0x00);
	response.push_back(0xA7);
	response.push_back(0x00);
	response.push_back('1');
	response.push_back(0x00);
	response.push_back(0x00);
	response.push_back(0x00);
	std::string protocol = std::to_string(PROTOCOL_VERSION);
	for (int16_t i = 0; i < protocol.size(); i++) {
		response.push_back(protocol[i]);
		response.push_back(0);
	}
	response.push_back(0);
	response.push_back(0);
	std::string gameVersion = std::string("b1.7.3");
	for (int16_t i = 0; i < gameVersion.size(); i++) {
		response.push_back(gameVersion[i]);
		response.push_back(0);
	}
	response.push_back(0);
	response.push_back(0);
	std::string motd = std::string("A Minecraft Server");
	for (int16_t i = 0; i < motd.size(); i++) {
		response.push_back(motd[i]);
		response.push_back(0);
	}
	response.push_back(0);
	response.push_back(0);
	std::string connectedPlayerCount = std::to_string(Betrock::Server::Instance().GetConnectedClients().size());
	for (int16_t i = 0; i < connectedPlayerCount.size(); i++) {
		response.push_back(connectedPlayerCount[i]);
		response.push_back(0);
	}
	response.push_back(0);
	response.push_back(0);
	std::string maxPlayerCount = "0";
	for (int16_t i = 0; i < maxPlayerCount.size(); i++) {
		response.push_back(maxPlayerCount[i]);
		response.push_back(0);
	}

	SendResponse(true);
	SetConnectionStatus(ConnectionStatus::Disconnected);
}

// Handle the latest server-bound packet from the client
void Client::HandlePacket() {
	auto serverTime = Betrock::Server::Instance().GetServerTime();
	int64_t lastPacketTime = serverTime;
	bool validPacket = true;
	// Prep for next packet
	ssize_t bytes_received = Setup();

	// If we receive no Data from the player, such as when they 
	if (bytes_received <= 0) {
		perror("read");
		DisconnectClient("No data.");
		return;
	}

	lastPacketTime = serverTime;

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
					HandleLoginRequest();
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

	SendResponse(true);
	BroadcastToClients(broadcastResponse);
	BroadcastToClients(broadcastOthersResponse, this);
	
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
		server.GetSpawnPoint(),
		server.GetSpawnDimension(),
		server.GetSpawnWorld(),
		server.GetSpawnPoint(),
		server.GetSpawnDimension(),
		server.GetSpawnWorld()
	);
	ClearInventory();

	// While the player is connected, read packets from them
	while (server.IsAlive() && GetConnectionStatus() > ConnectionStatus::Disconnected) {
		HandlePacket();
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

// --- Packet answers ---
// Respond to any KeepAlive packets
bool Client::HandleKeepAlive() {
	Respond::KeepAlive(response);
	return true;
}

// Engage with the handshake packet
bool Client::HandleHandshake() {
	player->username = EntryToString16(message, offset);
	Respond::Handshake(response);
	SetConnectionStatus(ConnectionStatus::LoggingIn);
	return true;
}

// Log the player in and perform checks to ensure a valid connection
bool Client::HandleLoginRequest() {
	bool firstJoin = true;
	auto &server = Betrock::Server::Instance();
	if (GetConnectionStatus() != ConnectionStatus::LoggingIn) {
		DisconnectClient("Expected Login.");
		return false;
	}

	// Login response
	int protocolVersion = EntryToInteger(message,offset);
	std::string username = EntryToString16(message,offset);
	EntryToLong(message,offset); // Get map seed
	EntryToByte(message,offset); // Get dimension

	if (protocolVersion != PROTOCOL_VERSION) {
		// If client has wrong protocol, close
		DisconnectClient("Wrong Protocol Version!");
		return false;
	}

	if (username != player->username) {
		DisconnectClient("Client has mismatched username.");
		return false;
	} 

	// Fill the players inventory
	if (player->Load()) {
		firstJoin = false;
	}

	// Accept the Login
	Respond::Login(response,player->entityId,1,0);
	Betrock::Logger::Instance().Info(username + " logged in with entity id " + std::to_string(player->entityId) + " at (" + std::to_string(player->position.x) + ", " + std::to_string(player->position.y) + ", " + std::to_string(player->position.z) + ")");
	Respond::ChatMessage(broadcastResponse, "§e" + username + " joined the game.");

  	const auto &spawnPoint = server.GetSpawnPoint();

	// Set the Respawn Point, Time and Player Health
	Respond::SpawnPoint(response,Vec3ToInt3(spawnPoint));
	Respond::Time(response,server.GetServerTime());
	// This is usually only done if the players health isn't full upon joining
	if (player->health != HEALTH_MAX) {
		Respond::UpdateHealth(response,player->health);
	}

	if (firstJoin) {
		// Place the player at spawn
		player->position = spawnPoint;

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

	// TODO: This hack seems stupid
	player->position.y += 0.1;
	// Note: Teleporting automatically loads surrounding chunks,
	// so no further loading is necessary
	Teleport(response,player->position, player->yaw, player->pitch);

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
	Respond::EntityTeleport(
		broadcastOthersResponse,
		player->entityId,
		Vec3ToEntityInt3(player->position),
		ConvertFloatToPackedByte(player->yaw),
		ConvertFloatToPackedByte(player->pitch)
	);

	// Spawn the other players for the new client
    for (auto other : Betrock::Server::Instance().GetConnectedClients()) {
		if (other.get() == this) { continue; }
		auto otherPlayer = other->GetPlayer();
		Respond::NamedEntitySpawn(
			response,
			otherPlayer->entityId,
			otherPlayer->username,
			Vec3ToInt3(otherPlayer->position),
			otherPlayer->yaw,
			otherPlayer->pitch,
			other->GetHeldItem().id
		);

		// Note: Even though we already send a packet that
		// tells the client what item the player holds,
		// this packet needs to be sent regardless
		// because otherwise the sky inverts
		Respond::EntityEquipment(
			response,
			otherPlayer->entityId,
			0,
			other->GetHeldItem().id,
			other->GetHeldItem().damage
		);
		
		// Apparently needed to entities show up where they need to
		Respond::EntityTeleport(
			response,
			otherPlayer->entityId,
			Vec3ToEntityInt3(otherPlayer->position),
			ConvertFloatToPackedByte(otherPlayer->yaw),
			ConvertFloatToPackedByte(otherPlayer->pitch)
		);

    }
	Respond::ChatMessage(response, std::string("This Server runs on ") + std::string(PROJECT_NAME_VERSION_FULL));
	SendResponse(true);
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
		Command::Parse(command, this);
	} else {
		std::string sentChatMessage = "<" + player->username + "> " + chatMessage;
		Betrock::Logger::Instance().Info(sentChatMessage);
		Respond::ChatMessage(broadcastResponse,sentChatMessage);
	}
	return true;
}

// Handle any interactions between entities
bool Client::HandleUseEntity() {
	int32_t originEntityId = EntryToInteger(message, offset);
	int32_t recipientEntityId = EntryToInteger(message, offset);
	bool leftClick = EntryToByte(message, offset);
	return true;
}

// Handle the player pressing the respawn button
bool Client::HandleRespawn() {
	int8_t dimension = EntryToByte(message, offset);
	Respawn(response);
	return true;
}

// Handle when the Client claims to be standing on solid ground
bool Client::HandlePlayerGrounded() {
	player->onGround = EntryToByte(message, offset);
	return true;
}

// Update the position of oneself for other clients
bool Client::UpdatePositionForOthers(bool includeLook) {
	Respond::EntityTeleport(
		broadcastOthersResponse,
		player->entityId,
		Vec3ToEntityInt3(player->position),
		ConvertFloatToPackedByte(player->yaw),
		ConvertFloatToPackedByte(player->pitch)
	);
	player->previousPosition = player->position;
	/*
	if (GetDistance(player->position,player->lastTickPosition) > 4.0) {
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

// Handle Player Position packets and relay this information to other clients
bool Client::HandlePlayerPosition() {
	Vec3 newPosition;
	double newStance;
	newPosition.x = EntryToDouble(message,offset);
	newPosition.y = EntryToDouble(message,offset);
	newStance = EntryToDouble(message,offset);
	newPosition.z = EntryToDouble(message,offset);
	player->onGround = EntryToByte(message, offset);
	CheckPosition(newPosition,newStance);
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
	int8_t face = EntryToByte(message, offset);

	Int3 pos = XyzToInt3(x,y,z);
	Block* targetedBlock = world->GetBlock(pos);

	// Check if the targeted block is interactable
	if (IsInteractable(targetedBlock->type)) {
		InteractWithBlock(targetedBlock);
		Respond::BlockChange(broadcastResponse,pos,targetedBlock->type,targetedBlock->meta);
		return true;
	}

	// If the block is broken or instantly breakable
	if (status == 2 || player->creativeMode || IsInstantlyBreakable(targetedBlock->type)) {
		Respond::BlockChange(broadcastResponse,pos,0,0);

		Respond::Soundeffect(broadcastOthersResponse,BLOCK_BREAK,pos,targetedBlock->type);
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
			Item item = Item{targetedBlock->type,1,targetedBlock->meta};
			if (!player->creativeMode) {
				item = GetDrop(item);
			}
			Give(response,item.id,item.amount,item.damage);
		}
		// Only get rid of the block here to avoid unreferenced pointers
		world->BreakBlock(pos);
	}
	return true;
}

// Check if the block position is intersecting with the player
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

// Handle the client attempting to place a block
bool Client::HandlePlayerBlockPlacement(World* world) {
	int32_t x = EntryToInteger(message, offset);
	int8_t y = EntryToByte(message, offset);
	int32_t z = EntryToInteger(message, offset);
	int8_t face = EntryToByte(message, offset);
	int16_t id = EntryToShort(message, offset);
	int8_t amount = 0;
	int16_t damage = 0;
	if (id >= 0) {
		amount = EntryToByte(message, offset);
		damage = EntryToShort(message, offset);
	}


	Int3 pos = XyzToInt3(x,y,z);
	Block* targetedBlock = world->GetBlock(pos);

	// Check if the targeted block is interactable
	if (IsInteractable(targetedBlock->type)) {
		InteractWithBlock(targetedBlock);
		Respond::BlockChange(broadcastResponse,pos,targetedBlock->type,targetedBlock->meta);
		return true;
	}
	BlockToFace(x,y,z,face);
	pos = XyzToInt3(x,y,z);

	// This packet has a special case where X, Y, Z, and Direction are all -1.
	// This special packet indicates that the currently held item for the player should have
	// its state updated such as eating food, shooting bows, using buckets, etc.

	// Apparently this also handles the player standing inside the block its trying to place in
	if (x == -1 && y == -1 && z == -1 && face == -1) {
		return false;
	}

	// Place a block if we can
	if (CanDecrementHotbar()) {
		// Check if the server-side inventory item is valid
		Item i = player->inventory[INVENTORY_HOTBAR+currentHotbarSlot];
		// Get the block we need to place
		Block b = GetPlacedBlock(x,y,z,face,GetPlayerOrientation(),i.id,i.damage);
		if (b.type == SLOT_EMPTY) {
			return false;
		}
		Respond::BlockChange(broadcastResponse,pos,b.type,b.meta);
		world->PlaceBlock(pos,b.type,b.meta);
		// Immediately give back the item if we're in creative mode
		if (player->creativeMode) {
			Item i = GetHeldItem();
			id = i.id;
			amount = i.amount;
			Respond::SetSlot(response,0,GetHotbarSlot(),id,amount,i.damage);
		} else {
			DecrementHotbar(response);
		}
	}
	return true;
}

// Handle the Client closing a Window
bool Client::HandleCloseWindow() {
	int8_t window 	= EntryToByte(message, offset);
	activeWindow = INVENTORY_NONE;
	return true;
}

// Handle the Client clicking while a Window is open
bool Client::HandleWindowClick() {
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
	ClickedSlot(response,window,slot,(bool)rightClick,actionNumber,shift,itemId,itemCount,itemUses);
	return true;
}

// Handle the Client attempting to disconnect
bool Client::HandleDisconnect() {
	std::string disconnectMessage = EntryToString16(message, offset);
	DisconnectClient(disconnectMessage);
	return true;
}

// This should be used for disconnecting clients
// Disconnect the current client from the server
void Client::DisconnectClient(std::string disconnectMessage) {
	SetConnectionStatus(ConnectionStatus::Disconnected);
	Respond::Disconnect(response, disconnectMessage);
	SendResponse(true);
	// Inform other clients
	Betrock::Logger::Instance().Info(player->username + " has disconnected. (" + disconnectMessage + ")");
	Respond::DestroyEntity(broadcastOthersResponse,player->entityId);
	Respond::ChatMessage(broadcastOthersResponse, "§e" + player->username + " left the game.");
	BroadcastToClients(broadcastOthersResponse,this);
}

// Add something to the current clients upcoming response packet
void Client::AppendResponse(std::vector<uint8_t> &addition) {
	if (!addition.empty()) {
		std::lock_guard<std::mutex> lock(responseMutex);
		response.insert(response.end(), addition.begin(), addition.end());
		SendResponse(true);
	}
}

// Send the contents of the response packet to the Client
void Client::SendResponse(bool autoclear) {
	if (response.empty() || GetConnectionStatus() <= ConnectionStatus::Disconnected) {
		return;
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
    player->yaw = yaw;
    player->pitch = pitch;
    player->stance = player->position.y + STANCE_OFFSET;
    newChunks.clear();
    Respond::PlayerPositionLook(response, player.get());
    DetermineVisibleChunks(true);
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
void Client::ClickedSlot(std::vector<uint8_t> &response, int8_t windowId, int16_t slotId, bool rightClick, int16_t actionNumber, bool shift, int16_t id, int8_t amount, int16_t damage) {
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
        return;
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
    i->amount--;
    if (i->amount <= 0) {
        i->id = -1;
        i->amount = 0;
        i->damage = 0;
    }
	Respond::SetSlot(response, 0, GetHotbarSlot(), i->id, i->amount, i->damage);
}

// Check if the passed chunk position is visible to the client
bool Client::ChunkIsVisible(Int3 pos) {
	return std::find(visibleChunks.begin(), visibleChunks.end(), pos) != visibleChunks.end();
}