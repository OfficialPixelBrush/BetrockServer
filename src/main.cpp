#include "main.h"

void PrepareForShutdown() {
    alive = false;
	// Save all active worlds
	if (!debugDisableSaveLoad) {
		for (auto& [key, wm] : worldManagers) {
			wm->world.Save(ConvertIndexIntoExtra(key));
		}
	}
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
		Player* player = new Player(client_fd, latestEntityId, spawnPoint, spawnWorld, spawnPoint, spawnWorld);
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
	std::srand(static_cast<unsigned int>(std::time(0)));
    const std::string filename = "server.properties";
    const std::unordered_map<std::string, std::string> defaultValues = {
		{"level-name","world"},
		{"view-distance","5"},
		// {"white-list","false"},
        {"server-ip", ""},
		//{"pvp","true"},
		{"level-seed",std::to_string(std::rand())},
		//{"spawn-animals",true}
        {"server-port", "25565"},
		//{"allow-nether",true},
		//{"spawn-monsters","true"},
		//{"max-players","20"},
		//{"online-mode","false"},
		//{"allow-flight","false"}
        {"generator", "terrain/perlin.lua"}
    };
    if (!std::filesystem::exists(filename)) {
        CreateDefaultProperties(filename, defaultValues);
    }
	properties = ReadPropertiesFile(filename);
	chunkDistance = std::stoll(properties["view-distance"]); 
	int64_t seed = std::stoll(properties["level-seed"]);
	std::cout << "Level seed is " << seed << std::endl;

	// Load all defined worlds
	// TODO: Add file to configure custom worlds
	AddWorldManager(0);
    for (auto& [key, wm] : worldManagers) {
		wm->SetSeed(seed);
		if (!debugDisableSaveLoad) {
        	wm->world.Load(ConvertIndexIntoExtra(key));
		}
    }
	
	WritePropertiesFile(filename,properties);
}

int main() {
	signal(SIGINT, HandleSignal);  // Handle Ctrl+C
	signal(SIGTERM, HandleSignal); // Handle termination signals

	std::cout << "Starting " << PROJECT_NAME << " version " << PROJECT_VERSION_STRING << std::endl;

	LoadConfig();

	const int port = std::stoi(properties["server-port"]);
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

	WorldManager* wm = GetWorldManager(0);
	World* overworld = GetWorld(0);

	// Generate spawn area
	if (overworld->GetNumberOfChunks() == 0) {
		std::cout << "Generating..." << std::endl;
		for (int x = -1; x < 2; x++) {
			for (int z = -1; z < 2; z++) {
				wm->AddChunkToQueue(x,z);
			}
		}
		//std::cout << "Generated " << newChunks << " Chunks" << std::endl;
	}
	while(!wm->QueueIsEmpty()) {
		// Wait for chunks to finish loading
	}
	// TODO: Wait for queue to finish
	Int3 spawnBlock = overworld->FindSpawnableBlock(Int3 {0,64,0});
	spawnPoint = Int3ToVec3(spawnBlock);
	spawnPoint.y+=STANCE_OFFSET;

    // Create threads for sending and receiving data
	std::thread join_thread(ServerJoin, address);
	std::vector<uint8_t> response;

	while (alive) {
		response.clear();
		// Server is alive
		if (doDaylightCycle) {
			serverTime += 20;
		}
		Respond::Time(response,serverTime);
		BroadcastToPlayers(response);
		sleep(1); // Send data every second
	}
	join_thread.join();
	PrepareForShutdown();
	return 0;
}
