#include "client.h"

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

bool ChunkBorderCrossed(Player* player) {
	if (BlockToChunkPosition(player->previousPosition) == BlockToChunkPosition(player->position)) {
		return false;
	}
	return true;
}

size_t SendChunksAroundPlayer(std::vector<uint8_t> &response, Player* player) {
	size_t numberOfNewChunks = 0;
	
	// Determine the center chunk from which to start from
	Int3 centerPos = Vec3ToInt3(player->position);
	Int3 playerChunkPos = BlockToChunkPosition(centerPos);
	int32_t pX = centerPos.x >> 4;
	int32_t pZ = centerPos.z >> 4;

	// Use an iterator to safely erase while iterating
	for (auto it = player->visibleChunks.begin(); it != player->visibleChunks.end(); ) {
		double distance = GetDistance(playerChunkPos, *it);
		if (distance > chunkDistance) {
			Respond::PreChunk(response, it->x, it->z, 0);
			it = player->visibleChunks.erase(it);
		} else {
			++it;
		}
	}

	// Go over all chunk coordinates around the center position
	for (int x = pX + chunkDistance*-1; x < pX + chunkDistance; x++) {
		for (int z = pZ + chunkDistance*-1; z < pZ + chunkDistance; z++) {
			World* world = GetDimension(player->dimension);
			Int3 position = XyzToInt3(x,0,z);

			// Acquire existing chunk data
			std::vector<uint8_t> chunkData = world->GetChunkData(position);
			if (chunkData.empty()) {
				// If none exists, generate new chunks
				world->GenerateChunk(x,z);
				numberOfNewChunks++;
				chunkData = world->GetChunkData(position);
			}
			// Add it to the list of chunks to be compressed and sent
			if (chunkData.empty()) {
				std::cout << "Failed to generated Chunk (" << (int)x << ", " << (int)z << ")" << std::endl;
				continue;
			}
			
			Respond::PreChunk(response, x, z, 1);

			size_t compressedSize = 0;
			char* chunk = CompressChunk(chunkData, compressedSize);
			
			if (chunk) {				
				// Track newly visible chunks
				player->visibleChunks.push_back(position);

				// Send compressed chunk data
				Respond::Chunk(
					response, 
					Int3{position.x << 4, 0, position.z << 4}, 
					CHUNK_WIDTH_X - 1, 
					CHUNK_HEIGHT - 1, 
					CHUNK_WIDTH_Z - 1, 
					compressedSize, 
					chunk
				);
			}
			delete [] chunk;
		}
	}
	SendToPlayer(response, player);
	response.clear();

	return numberOfNewChunks;
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
		Packet packetType = (Packet)EntryToByte(client.message,client.offset);

		// Provide debug info
		if (debugReceivedBytes || debugReceivedPacketType) {
			client.PrintReceived(packetType,bytes_received);
		}

		// Get the current Dimension
		World* world = GetDimension(client.player->dimension);
		
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
			case Packet::WindowClick:
				client.WindowClick();
				break;
			case Packet::Disconnect:
				client.DisconnectClient();
				break;
			default:
				break;
		}
		if (client.player != nullptr) {
			if (debugPlayerStatus) {
				client.player->PrintStats();
			}
			// Kill player if the goes below 0,0
			if (client.player->position.y < 0) {
				Respond::UpdateHealth(client.response,0);
			}
		}
		if (debugReceivedRead) {
			client.PrintRead(packetType);
		}
	}
	client.Respond(bytes_received);
}

// Give each Player their own thread
void HandleClient(Player* player) {
	// Assign player
	Client client = Client(player);

	// While the player is connected, read packets from them
	while (player->connectionStatus > ConnectionStatus::Disconnected) {
		HandlePacket(client);
	}
	std::lock_guard<std::mutex> lock(connectedPlayersMutex);
	int clientFdToDisconnect = player->client_fd;
    auto it = std::find(connectedPlayers.begin(), connectedPlayers.end(), player);
    if (it != connectedPlayers.end()) {
        connectedPlayers.erase(it);
    }

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
		std::cout << "Client has incorrect Protocol " << protocolVersion << "!" << std::endl;
		Disconnect(player,"Wrong Protocol Version!");
		return false;
	}
	// Accept the Login
	Respond::Login(response,player->entityId,1,0);
	std::cout << username << " logged in with entity id " << player->entityId << " at (" << player->position.x << ", " << player->position.y << ", " << player->position.z << ")" << std::endl;

	Respond::ChatMessage(broadcastResponse, "§e" + username + " joined the game.", 0);

	// Set the Respawn Point, Time and Player Health
	Respond::SpawnPoint(response,Vec3ToInt3(spawnPoint));
	Respond::Time(response,serverTime);
	Respond::UpdateHealth(response,player->health);

	// Fill the players inventory
	Respond::SetSlot(response,0,36,ITEM_PICKAXE_DIAMOND	, 1,0);
	Respond::SetSlot(response,0,37,ITEM_AXE_DIAMOND		, 1,0);
	Respond::SetSlot(response,0,38,ITEM_SHOVEL_DIAMOND	, 1,0);
	Respond::SetSlot(response,0,39,BLOCK_STONE			,64,0);
	Respond::SetSlot(response,0,40,BLOCK_COBBLESTONE	,64,0);
	Respond::SetSlot(response,0,41,BLOCK_PLANKS			,64,0);

	// Place the player at spawn
	// Note: Teleporting automatically loads surrounding chunks,
	// so no further loading is necessary
	player->Teleport(response,spawnPoint);

	// Create the player for other players
	Respond::NamedEntitySpawn(broadcastOthersResponse, player->entityId, player->username, Vec3ToInt3(player->position), player->yaw, player->pitch, BLOCK_PLANKS);

    for (Player* others : connectedPlayers) {
		if (others == player) { continue; }
		Respond::NamedEntitySpawn(response, others->entityId, others->username, Vec3ToInt3(others->position), others->yaw, others->pitch, BLOCK_PLANKS);
    }
	player->connectionStatus = ConnectionStatus::Connected;
	Respond::ChatMessage(response, std::string("This Server runs on ") + std::string(PROJECT_NAME_VERSION), 0);
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
		Vec3ToCompressedInt3(player->position),
		ConvertFloatToPackedByte(player->yaw),
		ConvertFloatToPackedByte(player->pitch)
	);

	if (ChunkBorderCrossed(player)) {
		SendChunksAroundPlayer(response,player);
	}

	// TODO: Figure this out!!
	/*
	if (GetDistance(player->previousPosition,player->position) < 4.0) {
		// If the movement was less than 4 blocks, just move
		Int3 difference = Vec3ToRelativeInt3(player->position, player->previousPosition);
		Respond::EntityRelativeMove(broadcastOthersResponse, player->entityId, difference);
	} else {
		// Otherwise, teleport the entity
		Respond::EntityTeleport(
			broadcastOthersResponse,
			player->entityId,
			Vec3ToInt3(player->position),
			ConvertFloatToPackedByte(player->yaw),
			ConvertFloatToPackedByte(player->pitch)
		);
	}*/
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
	if (ChunkBorderCrossed(player)) {
		SendChunksAroundPlayer(response,player);
	}
	return true;
}

bool Client::HoldingChange() {
	int16_t slotId = EntryToShort(message, offset);
	return true;
}

bool Client::Animation() {
	int32_t entityId = EntryToInteger(message, offset);
	int8_t animation = EntryToByte(message, offset);
	// TODO: Only send this to other clients
	//Respond::Animation(broadcastResponse, entityId, animation);
	return true;
}

bool Client::EntityAction() {
	int32_t entityId = EntryToInteger(message, offset);
	int8_t action = EntryToByte(message, offset);
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
	return true;
}

bool Client::PlayerDigging(World* world) {
	int8_t status = EntryToByte(message, offset);
	int32_t x = EntryToInteger(message, offset);
	int8_t y = EntryToByte(message, offset);
	int32_t z = EntryToInteger(message, offset);
	int8_t face = EntryToByte(message, offset);

	Int3 pos = XyzToInt3(x,y,z);
	// TODO: Figure out why this lags and sometimes doesn't work?
	// Maybe because I don't parse multi-packets?
	if (status == 2 || player->creativeMode) {
		Respond::BlockChange(broadcastResponse,pos,0,0);
		world->BreakBlock(pos);
	}
	return true;
}

bool Client::PlayerBlockPlacement(World* world) {
	int32_t x = EntryToInteger(message, offset);
	int8_t y = EntryToByte(message, offset);
	int32_t z = EntryToInteger(message, offset);
	int8_t direction = EntryToByte(message, offset);
	int16_t id = EntryToShort(message, offset);
	if (id >= 0) {
		int8_t amount = EntryToByte(message, offset);
		int16_t damage = EntryToShort(message, offset);
	}
	
	// If you don't catch this, the Server will try to
	// place a block with the Id of an empty slot, aka -1
	if (id > BLOCK_AIR && id < BLOCK_MAX) {
		BlockToFace(x,y,z,direction);
		Int3 pos = XyzToInt3(x,y,z);
		Respond::BlockChange(broadcastResponse,pos,(int8_t)(id&0xFF),0);
		world->PlaceBlock(pos,id);
	}
	return true;
}

bool Client::WindowClick() {
	int8_t windowId 	= EntryToByte(message, offset);
	int16_t slotId 		= EntryToShort(message,offset);
	int8_t rightClick 	= EntryToByte(message, offset);
	int16_t actionNumber= EntryToShort(message,offset);
	int8_t shift 		= EntryToByte(message, offset);
	int16_t itemId		= EntryToShort(message,offset);
	if (itemId > 0) {
		int8_t itemCount	= EntryToByte(message, offset);
		int16_t itemUses	= EntryToShort(message,offset);
	}
	return true;
}

bool Client::DisconnectClient() {
	Disconnect(player);
	Respond::ChatMessage(broadcastResponse, "§e" + player->username + " left the game.");
	return true;
}