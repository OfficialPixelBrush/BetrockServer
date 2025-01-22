#include "client.h"

// Give each Player their own thread
void HandleClient(Player* player) {
	int64_t lastPacketTime = serverTime;
	// While the player is connected, tick them
	while (player->connectionStatus > Disconnected) {
		std::vector<uint8_t> response;
		std::vector<uint8_t> broadcastResponse;
		char message[4096] = {0};
		int32_t offset = 0;

		ssize_t bytes_received = read(player->client_fd, message, 4096);
		
		// If we receive no Data from the player, such as when they 
		if (bytes_received <= 0) {
			perror("read");
			Disconnect(player,"No data.");
			break;
		}

		lastPacketTime = serverTime;

		// TODO: figure out how to read packet bundles!!
		uint8_t packetType = EntryToByte(message,offset);
		
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

		// Get the current Dimension
		World* world = GetDimension(player->dimension);
		
		// The Client tries to join the Server
		switch(packetType) {
			case 0x00: // Keep Alive
				Respond::KeepAlive(response);
				break;
			case 0x01: {// Login
				if (player->connectionStatus != LoggingIn) {
					Disconnect(player,"Expected Login.");
					break;
				}

				// Login response
				int protocolVersion = EntryToInteger(message,offset);
				std::string username = EntryToString16(message,offset); // Get username again
				if (username != player->username) {
					Disconnect(player,"Client has mismatched username.");
					break;
				} 
				EntryToLong(message,offset); // Get map seed
				EntryToByte(message,offset); // Get dimension

				if (protocolVersion != PROTOCOL_VERSION) {
					// If client has wrong protocol, close
					std::cout << "Client has incorrect Protocol " << protocolVersion << "!" << std::endl;
					Disconnect(player,"Wrong Protocol Version!");
					break;
				}
				// Accept the Login
				Respond::Login(response,player->entityId,1,0);
				std::cout << username << " logged in with entity id " << player->entityId << " at (" << player->position.x << ", " << player->position.y << ", " << player->position.z << ")" << std::endl;

				Respond::ChatMessage(broadcastResponse, "§e" + username + " joined the game.", 0);

				// Set the Respawn Point, Time and Player Health
				Respond::SpawnPoint(response,spawnPoint);
				Respond::Time(response,serverTime);
				Respond::UpdateHealth(response,player->health);

				// TODO: Lazy-load chunks
				SendChunksAroundPlayer(response,player);

				// Fill the players inventory
				Respond::SetSlot(response,0,36,ITEM_PICKAXE_DIAMOND	, 1,0);
				Respond::SetSlot(response,0,37,ITEM_AXE_DIAMOND		, 1,0);
				Respond::SetSlot(response,0,38,ITEM_SHOVEL_DIAMOND	, 1,0);
				Respond::SetSlot(response,0,39,BLOCK_STONE			,64,0);
				Respond::SetSlot(response,0,40,BLOCK_COBBLESTONE	,64,0);
				Respond::SetSlot(response,0,41,BLOCK_PLANKS			,64,0);

				// This is when the player starts to see the world!
				Respond::PlayerPositionLook(response,player);
				Respond::ChatMessage(response, std::string("This Server runs on ") + std::string(PROJECT_NAME_VERSION), 0);
				player->connectionStatus = Connected;
				break;
			}
			case 0x02: { // Handshake
				if (player->connectionStatus != Handshake) {
					Disconnect(player,"Expected Handshake.");
					break;
				}
				player->username = EntryToString16(message, offset);
				Respond::Handshake(response);
				player->connectionStatus = LoggingIn;
				break;
			}
			case 0x03: { // Chat Message
				std::string chatMessage = EntryToString16(message, offset);
				if (chatMessage.size() > 0 && chatMessage[0] == '/') {
					std::string command = chatMessage.substr(1);
					Command::Parse(command, player);
				} else {
					std::string sentChatMessage = "<" + player->username + "> " + chatMessage;
					Respond::ChatMessage(broadcastResponse,sentChatMessage);
				}
				break;
			}
			case 0x09: { // Respawn
				int8_t dimension = EntryToByte(message, offset);
    			player->Respawn(response);
				break;
			}
			case 0x0A: // Player
				player->onGround = EntryToByte(message, offset);
				break;
			case 0x0B: // Player Position
				player->position.x = EntryToDouble(message,offset);
				player->position.y = EntryToDouble(message,offset);
				player->stance = EntryToDouble(message,offset);
				player->position.z = EntryToDouble(message,offset);
				player->onGround = EntryToByte(message, offset);
				break;
			case 0x0C: // Player Look
				player->yaw = EntryToFloat(message,offset);
				player->pitch = EntryToFloat(message,offset);
				player->onGround = EntryToByte(message, offset);
				break;
			case 0x0D: // Player Position Look
				player->position.x = EntryToDouble(message,offset);
				player->position.y = EntryToDouble(message,offset);
				player->stance = EntryToDouble(message,offset);
				player->position.z = EntryToDouble(message,offset);
				player->yaw = EntryToFloat(message,offset);
				player->pitch = EntryToFloat(message,offset);
				player->onGround = EntryToByte(message, offset);
				//Respond::PlayerPositionLook(response,player);
				break;
			case 0x12: { // Animation
				int32_t entityId = EntryToInteger(message, offset);
				int8_t animation = EntryToByte(message, offset);
				Respond::Animation(broadcastResponse, entityId, animation);
				break;
			}
			case 0x13: // Entity Action
				{
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
				break;
				}
			case 0x0E: { // Player Digging
				int8_t status = EntryToByte(message, offset);
				int32_t x = EntryToInteger(message, offset);
				int8_t y = EntryToByte(message, offset);
				int32_t z = EntryToInteger(message, offset);
				int8_t face = EntryToByte(message, offset);

				Int3 pos = XyzToInt3(x,y,z);
				if (status == 2 || player->creativeMode) {
					Respond::BlockChange(broadcastResponse,pos,0,0);
					world->BreakBlock(pos);
				}
				break;
			}
			case 0x0F: { // Player Place Block
				int32_t x = EntryToInteger(message, offset);
				int8_t y = EntryToByte(message, offset);
				int32_t z = EntryToInteger(message, offset);
				int8_t direction = EntryToByte(message, offset);
				int16_t id = EntryToShort(message, offset);
				int8_t amount = EntryToByte(message, offset);
				int16_t damage = EntryToShort(message, offset);
				
				// If you don't catch this, the Server will try to
				// place a block with the Id of an empty slot, aka -1
				if (id > BLOCK_AIR && id < BLOCK_MAX) {
					BlockToFace(x,y,z,direction);
					Int3 pos = XyzToInt3(x,y,z);
					Respond::BlockChange(broadcastResponse,pos,(int8_t)(id&0xFF),0);
					world->PlaceBlock(pos,id);
				}
				break;
			}
			case 0xFF: { // Disconnect
				player->connectionStatus = Disconnected;
				Respond::ChatMessage(broadcastResponse, "§e" + player->username + " left the game.");
				break;
			}
			default:
				break;
		}
		if (player != nullptr) {
			if (debugPlayerStatus) {
				player->PrintStats();
			}
			// Kill player if the goes below 0,0
			if (player->position.y < 0) {
				Respond::UpdateHealth(response,0);
			}
		}
		SendToPlayer(response, player);
		BroadcastToPlayers(broadcastResponse);
		if (debugNumberOfPacketBytes) {
			std::cout << "--- " << offset << "/" << bytes_received << " Bytes Read from Packet ---" << std::endl << std::endl; 
		}
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
}