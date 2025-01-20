#include "main.h"
int server_fd;  // Declare the server file descriptor globally
bool alive = true;
int connectingStage = 0;
int chunkX = -64;
int chunkZ = -64;

void handle_signal(int sig) {
	std::cout << "Shutting down server..." << std::endl;
	if (server_fd >= 0) close(server_fd);
	exit(0);
}

void SendToClient(std::vector<uint8_t> &response, int client_fd) {
	// Reply
	if (response.size() > 0) {
		std::cout << "Sending " << PacketIdToLabel(response[0]) << " to Client! (" << response.size() << " Bytes)" << std::endl;
		/*
		for (uint i = 0; i < response.size(); i++) {
			std::cout << std::hex << (int)response[i];
			if (i < response.size()-1) {
				std::cout << ", ";
			}
		}
		std::cout << std::dec << std::endl;
		*/
		ssize_t bytes_sent = send(client_fd, response.data(), response.size(), 0);
		if (bytes_sent == -1) {
			perror("send");
			return;
		}
	}
	std::cout << std::endl;
}

void Disconnect(std::vector<uint8_t> &response, int client_fd, std::string message) {
	std::cout << "Disconnecting with Message: " << message << std::endl;
	response.push_back(0xFF);
	appendString16ToVector(response,message);
	SendToClient(response,client_fd);
	connectingStage = 0;
}

void *send_data(void *arg) {
    int client_fd = *(int *)arg;
	bool sendingChunks = true;
    while (sendingChunks) {
		std::vector<uint8_t> response;
		RespondKeepAlive(response);
		SendToClient(response,client_fd);
        sleep(2); // Send data every second
    }
    return NULL;
}

int main() {
	std::cout << PROJECT_NAME_VERSION << " started!" << std::endl;
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

	listenForNew:
	// Listen for connections
	if (listen(server_fd, 3) < 0) {
		perror("Listen failed");
		return 1;
	}

	std::cout << "Server is listening on port " << port << std::endl;

	// Accept connections
	if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
		perror("Accept failed");
		return 1;
	}
    // Create threads for sending and receiving data
    //pthread_t send_thread, receive_thread;
    //pthread_create(&send_thread, NULL, send_data, &client_fd);
	bool respondingChunks = true;

	while (alive) {
		std::vector<uint8_t> response;
		char message[1024] = {0};
		ssize_t bytes_received = read(client_fd, message, 1024);
		
		if (bytes_received <= 0) {
			perror("read");
			break;
		}

		uint8_t packetType = message[0];
		std::cout << "Received " << PacketIdToLabel(packetType) << " from Client! (" << bytes_received << " Bytes)" << std::endl;
		for (uint i = 0; i < bytes_received; i+=2) {
			std::cout << std::hex << (int)message[i];
			if (i < bytes_received-2) {
				std::cout << ", ";
			}
		}
		std::cout << std::dec << std::endl;

		// The Client tries to join the Server
		switch(connectingStage) {
			case 0:
				if (packetType == 2) {
					// Handshake: extract the username
					std::string username(message + 1, bytes_received - 1);  // Skip the first byte (type)
					std::cout << username << " is trying to connect." << std::endl;
					RespondHandshake(response);
					connectingStage++;
				} else {
					Disconnect(response,client_fd,"Expected Handshake.");
				}
				break;
			case 1:
				if (packetType == 1) {
					// Login response
					int protocolVersion = (message[2] << 8) | message[4];
					if (protocolVersion == PROTOCOL_VERSION) {
						std::cout << "Client has correct Protocol!" << std::endl;
						RespondLogin(response,1,1,0);
						connectingStage++;
					} else {
						// If client has wrong protocol, close
						std::cout << "Client has incorrect Protocol " << protocolVersion << "!" << std::endl;
						Disconnect(response,client_fd,"Client has wrong version.");
					}
				} else {
					Disconnect(response,client_fd,"Expected Login.");
				}
				break;
			case 2:
				switch(packetType) {
					default:
						// Yell it back at the client?
						if (bytes_received > 0) {
							response.assign(message, message + bytes_received);
						}
						break;
					case 0x00:
						RespondKeepAlive(response);
						break;
					case 0x03: {
						uint8_t stringLength = message[2];
						for (uint8_t i = 4; i < stringLength*2+4; i+=2) {
							std::cout << message[i];
						}
						std::cout << std::endl;
						Disconnect(response,client_fd,"1984");
						//RespondChatMessage(response,message);
						break;
						}
					case 0xFF:
						std::cout << "Client has disconnected." << std::endl;
						return 1;
				}
				break;
			case 3:
				// Send the Spawn point
				RespondSpawnPoint(response,1,1,1);
				connectingStage++;
				break;
			case 4:
				// Send the Server Time
				RespondTime(response,0);
				connectingStage++;
				break;
			case 5:
				// Send the Player Health
				RespondUpdateHealth(response,20);
				connectingStage++;
				break;
			case 6:
				if (respondingChunks) {
					RespondChunk(response, chunkX, 0, chunkZ, 1, 127, 1);
					if (chunkX > 64) {
						chunkZ++;
						chunkX = -64;
					}
					if (chunkZ > 64) {
						respondingChunks = false;
					}
				}
				break;
			default:
				// Something fucked up massively
				std::cout << "Unknown message type: " << (int)message[0] << std::endl;
				Disconnect(response,client_fd,"ERROR!");
				goto listenForNew;
				break;
		}
		SendToClient(response, client_fd);
	}
    //pthread_join(send_thread, NULL);
    //pthread_join(receive_thread, NULL);


	close(client_fd);
	close(server_fd);
	return 0;
}
