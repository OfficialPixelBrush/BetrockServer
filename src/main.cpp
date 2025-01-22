#include "main.h"

void HandleSignal(int sig) {
	std::cout << "Shutting down server..." << std::endl;
	if (server_fd >= 0) close(server_fd);
	exit(0);
}

void *ServerJoin(void *arg) {
    struct sockaddr_in address = *(sockaddr_in *)arg;
	int addrlen = sizeof(address);
	int client_fd;
	while (alive) {
		// Accept connections
		if ((client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) < 0) {
			perror("Accept failed");
			return nullptr;
		}

		// Create new player
		std::lock_guard<std::mutex> lockEntityId(entityIdMutex);
		Player* player = new Player(client_fd, latestEntityId, Int3ToVec3(spawnPoint), spawnDimension, Int3ToVec3(spawnPoint), spawnDimension);
		player->connectionStatus = Handshake;

		// Add this new player to the list of connected Players
		std::lock_guard<std::mutex> lockConnectedPlayers(connectedPlayersMutex);
		connectedPlayers.push_back(player);
		
		// Let each player have their own thread
		std::thread playerThread(HandleClient, player);
		playerThread.detach();
	}
	return nullptr;
}

int main() {
	std::cout << "Starting " << PROJECT_NAME << " version " << PROJECT_VERSION_STRING << std::endl;
	signal(SIGINT, HandleSignal);  // Handle Ctrl+C
	signal(SIGTERM, HandleSignal); // Handle termination signals

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
    pthread_t join_thread;
	// Begin ticking time
	pthread_create(&join_thread, NULL, ServerJoin, &address);

	// Generate a basic world
    std::cout << "Generating..." << std::endl;
	uint newChunks = 0;
	for (int x = chunkDistance*-1; x < chunkDistance; x++) {
		for (int z = chunkDistance*-1; z < chunkDistance; z++) {
			overworld.GenerateChunk(x,z);
			newChunks++;
		}
	}
    std::cout << "Generated " << newChunks << " Overworld Chunks" << std::endl;
	spawnPoint = overworld.FindSpawnableBlock(Int3 {0,64,0});

	while (alive) {
		// Server is alive
		std::vector<uint8_t> response;
		serverTime += 20;
		Respond::Time(response,serverTime);
		BroadcastToPlayers(response);
        sleep(1); // Send data every second
	}
	DisconnectAllPlayers("Server closed!");
	pthread_join(join_thread, NULL);
	close(server_fd);
	return 0;
}
