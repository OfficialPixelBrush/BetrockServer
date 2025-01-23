#include "main.h"

void PrepareForShutdown() {
	alive = false;
	overworld.Save();
	nether.Save();
	DisconnectAllPlayers("Server closed!");
	close(server_fd);
}

void HandleSignal(int sig) {
	PrepareForShutdown();
    shutdown(server_fd, SHUT_RDWR); // Interrupt accept
	exit(0);
}

void ServerJoin(struct sockaddr_in address) {
	std::vector<std::thread> playerThreadPool;
	int addrlen = sizeof(address);
	int client_fd;
	
	while (alive) {
		// Accept connections
		client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
		if (client_fd < 0) {
			perror("Accept failed");
			continue;
		}

		// Create new player
		std::lock_guard<std::mutex> lockEntityId(entityIdMutex);
		Player* player = new Player(client_fd, latestEntityId, spawnPoint, spawnDimension, spawnPoint, spawnDimension);
		player->connectionStatus = ConnectionStatus::Handshake;

		// Add this new player to the list of connected Players
		std::lock_guard<std::mutex> lockConnectedPlayers(connectedPlayersMutex);
		connectedPlayers.push_back(player);
		
		// Let each player have their own thread
		// TODO: Make this non-cancerous, and close player threads upon disconnect
		playerThreadPool.emplace_back(HandleClient, player);
	}

    for (auto& thread : playerThreadPool) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void LoadConfig() {
	// TODO: Read server.properties etc.
	// TODO: Read level.dat
	overworld.Load();
	//nether.Load("DIM-1");
	overworld.SetSeed(1);
	//nether.SetSeed(1);
}

int main() {
	signal(SIGINT, HandleSignal);  // Handle Ctrl+C
	signal(SIGTERM, HandleSignal); // Handle termination signals

	std::cout << "Starting " << PROJECT_NAME << " version " << PROJECT_VERSION_STRING << std::endl;

	LoadConfig();

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
	std::thread join_thread(ServerJoin, address);

	// Generate spawn area
	if (overworld.GetNumberOfChunks() == 0) {
		std::cout << "Generating..." << std::endl;
		uint newChunks = 0;
		for (int x = -1; x < 2; x++) {
			for (int z = -1; z < 2; z++) {
				overworld.GenerateChunk(x,z);
				newChunks++;
			}
		}
		std::cout << "Generated " << newChunks << " Overworld Chunks" << std::endl;
	}
	Int3 spawnBlock = overworld.FindSpawnableBlock(Int3 {0,64,0});
	spawnPoint = Int3ToVec3(spawnBlock);
	spawnPoint.y+=STANCE_OFFSET;

	while (alive) {
		// Server is alive
		std::vector<uint8_t> response;
		serverTime += 20;
		Respond::Time(response,serverTime);
		BroadcastToPlayers(response);
        sleep(1); // Send data every second
	}
	PrepareForShutdown();
	join_thread.join();
	return 0;
}
