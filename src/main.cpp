#include "main.h"
int server_fd;  // Declare the server file descriptor globally
bool alive = true;
uint serverTime = 0;
int32_t latestEntityId = 0;
std::mutex entityIdMutex;

std::vector<int> connectedClients;
std::mutex connectedClientsMutex;

bool creativeMode = false;

void handle_signal(int sig) {
	std::cout << "Shutting down server..." << std::endl;
	if (server_fd >= 0) close(server_fd);
	exit(0);
}

void SendToClient(std::vector<uint8_t> &response, int client_fd) {
	// Reply
	if (response.size() > 0) {
		if (debugSentPacketType) {
			std::cout << "Sending " << PacketIdToLabel(response[0]) << " to Client! (" << response.size() << " Bytes)" << std::endl;
		}
			
		if (debugSentBytes) {
			for (uint i = 0; i < response.size(); i++) {
				std::cout << std::hex << (int)response[i];
				if (i < response.size()-1) {
					std::cout << ", ";
				}
			}
			std::cout << std::dec << std::endl;
		}
		
		ssize_t bytes_sent = send(client_fd, response.data(), response.size(), 0);
		if (bytes_sent == -1) {
			perror("send");
			return;
		}
	}
}

void BroadcastToClients(std::vector<uint8_t> &response) {
	std::lock_guard<std::mutex> lock(connectedClientsMutex);
    for (int client_fd : connectedClients) {
        SendToClient(response,client_fd);
    }
}

void Disconnect(std::vector<uint8_t> &response, int client_fd, std::string message) {
	std::cout << "Disconnecting with Message: " << message << std::endl;
	response.push_back(0xFF);
	appendString16ToVector(response,message);
	SendToClient(response,client_fd);
}

void *internalTicks(void *arg) {
    int client_fd = *(int *)arg;
    while (true) {
		std::vector<uint8_t> response;
		serverTime += 20;
		RespondTime(response,serverTime);
		BroadcastToClients(response);
        sleep(1); // Send data every second
    }
    return NULL;
}

// Parses commands and executes them
void ParseCommand(std::string& rawCommand, Player* player) {
	bool successful = false;
	std::vector<uint8_t> response;

    std::string s;
    std::stringstream ss(rawCommand);
    std::vector<std::string> command;

    while (getline(ss, s, ' ')) {
        // store token string in the vector
        command.push_back(s);
    }

    if (command[0] == "time") {
		if (command.size() > 1) {
			serverTime = std::stol(command[1].c_str());
			RespondTime(response,serverTime);
			SendToClient(response, player->client_fd);
			successful = true;
		}
    } else if (command[0] == "give") {
		if (command.size() > 1) {
			int itemId = std::stoi(command[1].c_str());
			RespondSetSlot(response,0,36,itemId,64,0);
			SendToClient(response, player->client_fd);
			return;
		}
	} else if (command[0] == "health") {
		if (command.size() > 1) {
			int health = std::stoi(command[1].c_str());
			if (health > 20) { health = 20; }
			if (health < 0) { health = 0; }
			RespondUpdateHealth(response,health);
			SendToClient(response, player->client_fd);
			successful = true;
		}
	} else if (command[0] == "kill") {
		RespondUpdateHealth(response,0);
		SendToClient(response, player->client_fd);
		successful = true;
	} else if (command[0] == "spawn") {
		if (command.size() > 1) {
			int mobId = std::stoi(command[1].c_str());
			RespondMobSpawn(response,latestEntityId,mobId,0,100,0,0,0);
			std::cout << "Spawned " << mobId << std::endl;
			SendToClient(response, player->client_fd);
			successful = true;
		}
	} else if (command[0] == "summon") {
		if (command.size() > 1) {
			std::lock_guard<std::mutex> lock(entityIdMutex);
			std::vector<uint8_t> broadcastResponse;
			RespondNamedEntitySpawn(broadcastResponse, latestEntityId, command[1], player->x, player->y, player->z, 0,0, 5);
			BroadcastToClients(broadcastResponse);
			successful = true;
		}
	} else if (command[0] == "creative") {
		creativeMode = !creativeMode;
		successful = true;
	} else if (command[0] == "stop") {
		alive = !alive;
		successful = true;
	}

	if (!successful) {
		RespondChatMessage(response, "§cInvalid Syntax \"" + rawCommand + "\"");
	} else {
		RespondChatMessage(response, "§7Executed \"" + rawCommand + "\"");
	}
	SendToClient(response,player->client_fd);
}

void *HandleClient(void *arg) {
    int client_fd = *(int *)arg;
	Player* player = nullptr;
	int connectingStage = 0;
	bool connected = true;
	bool broadcast = false;
	while (connected) {
		broadcast = false;
		std::vector<uint8_t> response;
		char message[4096] = {0};
		int32_t offset = 0;
		ssize_t bytes_received = read(client_fd, message, 4096);
		
		if (bytes_received <= 0) {
			perror("read");
			connected = false;
			break;
		}

		// TODO: figure out how to read packet bundles!!
		uint8_t packetType = entryToByte(message,offset);
		
		if (debugReceivedPacketType) {
			std::cout << "Received " << PacketIdToLabel(packetType) << " from Client! (" << bytes_received << " Bytes)" << std::endl;
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

		// The Client tries to join the Server
		switch(packetType) {
			case 0x00: // Keep Alive
				RespondKeepAlive(response);
				break;
			case 0x01: {// Login
				if (connectingStage != 1) {
					Disconnect(response,client_fd,"Expected Login.");
				}

				// Login response
				int protocolVersion = entryToInteger(message,offset);
				std::string username = entryToString16(message,offset); // Get username again
				entryToLong(message,offset); // Get map seed
				entryToByte(message,offset); // Get dimension

				if (protocolVersion != PROTOCOL_VERSION) {
					// If client has wrong protocol, close
					std::cout << "Client has incorrect Protocol " << protocolVersion << "!" << std::endl;
					Disconnect(response,client_fd,"Client has wrong version.");
					break;
				}
				// Accept the Login
				std::lock_guard<std::mutex> lock(entityIdMutex);
				RespondLogin(response,latestEntityId,1,0);
				latestEntityId--;

				player = new Player(latestEntityId,client_fd,username);
				std::cout << username << " logged in with entity id " << latestEntityId << " at (" << player->x << ", " << player->y << ", " << player->z << ")" << std::endl;
				latestEntityId++;

				std::vector<uint8_t> broadcastResponse;
				RespondChatMessage(broadcastResponse, "§e" + username + " joined the game.");
				BroadcastToClients(broadcastResponse);

				// Set the Respawn Point, Time and Player Health
				RespondSpawnPoint(response,0,127,0);
				RespondTime(response,0);
				RespondUpdateHealth(response,10);

				// Pre-load some chunks
				size_t compressedSize = 0;
				char* chunk = CompressChunk(GetChunk(0,0,0),compressedSize);

				for (int x = chunkRadius*-1; x < chunkRadius; x++) {
					for (int z = chunkRadius*-1; z < chunkRadius; z++) {
						RespondPreChunk(response,x,z,1);
						RespondChunk(response,x*16,0,z*16,15,127,15,compressedSize,chunk);
					}
				}

				delete[] chunk;

				// Fill the players inventory
				RespondSetSlot(response,0,37,5,64,0);
				//RespondWindowItems(response,0,20);

				// This is when the player starts to see the world!
				RespondPlayerPositionLook(response,player);
				RespondChatMessage(response, std::string("This Server runs on ") + std::string(PROJECT_NAME_VERSION));
				break;
			}
			case 0x02: { // Handshake
				if (connectingStage == 0) {
					entryToString16(message, offset);
					RespondHandshake(response);
					connectingStage++;
				} else {
					Disconnect(response,client_fd,"Expected Handshake.");
				}
				break;
			}
			case 0x03: { // Chat Message
				broadcast = true;
				std::string chatMessage = entryToString16(message, offset);
				if (chatMessage.size() > 0 && chatMessage[0] == '/') {
					std::string command = chatMessage.substr(1);
					ParseCommand(command, player);
				} else {
					std::string sentChatMessage = "<" + player->username + "> " + chatMessage;
					std::cout << sentChatMessage << std::endl;
					RespondChatMessage(response,sentChatMessage);
				}
				break;
			}
			case 0x0A: // Player
				player->onGround = entryToByte(message, offset);
				break;
			case 0x0B: // Player Position
				player->x = entryToDouble(message,offset);
				player->y = entryToDouble(message,offset);
				player->stance = entryToDouble(message,offset);
				player->z = entryToDouble(message,offset);
				player->onGround = entryToByte(message, offset);
				break;
			case 0x0C: // Player Look
				player->yaw = entryToFloat(message,offset);
				player->pitch = entryToFloat(message,offset);
				player->onGround = entryToByte(message, offset);
				break;
			case 0x0D: // Player Position Look
				player->x = entryToDouble(message,offset);
				player->y = entryToDouble(message,offset);
				player->stance = entryToDouble(message,offset);
				player->z = entryToDouble(message,offset);
				player->yaw = entryToFloat(message,offset);
				player->pitch = entryToFloat(message,offset);
				player->onGround = entryToByte(message, offset);
				break;
			case 0x12: { // Animation
				int32_t entityId = entryToInteger(message, offset);
				int8_t animation = entryToByte(message, offset);
				std::vector<uint8_t> broadcastResponse;
				RespondAnimation(broadcastResponse, entityId, animation);
				break;
			}
			case 0x13: // Entity Action
				{
				int32_t entityId = entryToInteger(message, offset);
				int8_t action = entryToByte(message, offset);
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
				int8_t status = entryToByte(message, offset);
				int32_t x = entryToInteger(message, offset);
				int8_t y = entryToByte(message, offset);
				int32_t z = entryToInteger(message, offset);
				int8_t face = entryToByte(message, offset);
				if (status == 2 || creativeMode) {
					RespondBlockChange(response,x,y,z,0,0);
					//RespondPlayerDigging(response,status,x,y,z,face);
				}

				std::vector<uint8_t> broadcastResponse;
				BroadcastToClients(broadcastResponse);
				broadcast = true;
				break;
			}
			case 0x0F: {
				int32_t x = entryToInteger(message, offset);
				int8_t y = entryToByte(message, offset);
				int32_t z = entryToInteger(message, offset);
				int8_t direction = entryToByte(message, offset);
				int16_t id = entryToShort(message, offset);
				int8_t amount = entryToByte(message, offset);
				int16_t damage = entryToShort(message, offset);

				blockToFace(x,y,z,direction);
				RespondBlockChange(response,x,y,z,(int8_t)(id&0xFF),0);
				broadcast = true;
				break;
			}
			case 0xFF: { // Disconnect
				std::cout << player->username << " has disconnected." << std::endl;
				std::vector<uint8_t> broadcastResponse;
				RespondChatMessage(broadcastResponse, "§e" + player->username + " left the game.");
				BroadcastToClients(broadcastResponse);
				connected= false;
				break;
			}
			default:
				break;
		}
		if (player != nullptr) {
			// TODO: Implement Flying
			/*
			if (creativeMode) {
				if (oldY < player->y && !player->crouching) {
					player->y = oldY;
					RespondPlayerPosition(response,player);
				}
			}*/

			if (debugPlayerStatus) {
				player->PrintStats();
			}
			// Kill player if the goes below 0,0
			if (player->y < 0) {
				RespondUpdateHealth(response,0);
			}
		}
		if (broadcast) {
			BroadcastToClients(response);
		} else {
			SendToClient(response, client_fd);
		}
		if (debugNumberOfPacketBytes) {
			std::cout << "--- " << offset << "/" << bytes_received << " Bytes Read from Packet ---" << std::endl << std::endl; 
		}
	}
	delete player;
	player = nullptr;
	std::lock_guard<std::mutex> lock(connectedClientsMutex);
    auto it = std::find(connectedClients.begin(), connectedClients.end(), client_fd);
    if (it != connectedClients.end()) {
        connectedClients.erase(it);
    }
	close(client_fd);
    return nullptr;
}

int main() {
	std::cout << "Starting " << PROJECT_NAME << " version " << PROJECT_VERSION_STRING << std::endl;
	signal(SIGINT, handle_signal);  // Handle Ctrl+C
	signal(SIGTERM, handle_signal); // Handle termination signals

	const int port = 25565;
	int client_fd;
	struct sockaddr_in address;
	int addrlen = sizeof(address);

	// Create socket
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		perror("Socket creation failed");
		return 1;
	}
	
	int opt = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt failed");
		return 1;
	}

	// Bind socket to port 25565
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("Bind failed");
		return 1;
	}

	// Listen for connections
	if (listen(server_fd, 3) < 0) {
		perror("Listen failed");
		return 1;
	}

	std::cout << "Starting " << PROJECT_NAME " on *:" << port << std::endl;
    // Create threads for sending and receiving data
    pthread_t tick_thread;
	// Begin ticking time
	pthread_create(&tick_thread, NULL, internalTicks, &client_fd);

	while (alive) {
		// Accept connections
		if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
			perror("Accept failed");
			return 1;
		}
    	pthread_t player_thread;
		pthread_create(&player_thread, NULL, HandleClient, &client_fd);
		std::lock_guard<std::mutex> lock(connectedClientsMutex);
		connectedClients.push_back(client_fd);
	}
	pthread_join(tick_thread, NULL);
	close(server_fd);
	return 0;
}
