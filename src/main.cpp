#include "main.h"

#include "server.h"

void __attribute__((noreturn)) HandleSignal(int) {
	Betrock::Server::Instance().PrepareForShutdown();
	shutdown(Betrock::Server::Instance().GetServerFd(), SHUT_RDWR); // Interrupt accept
	exit(0);
}

int main() {
	auto &server = Betrock::Server::Instance();

	signal(SIGINT, HandleSignal);  // Handle Ctrl+C
	signal(SIGTERM, HandleSignal); // Handle termination signals

	std::cout << "Starting " << PROJECT_NAME << " version " << PROJECT_VERSION_STRING << std::endl;

	server.LoadConfig();

	auto port = static_cast<uint16_t>(std::stoi(properties["server-port"]));
	std::cout << "Starting " << PROJECT_NAME " on *:" << port << std::endl;

	if (!server.SocketBootstrap(port)) {
		return EXIT_FAILURE;
	}

	WorldManager *wm = server.GetWorldManager(0);
	World *overworld = server.GetWorld(0);

	// Generate spawn area
	if (overworld->GetNumberOfChunks() == 0) {
		std::cout << "Generating..." << std::endl;
		for (int x = -1; x < 2; x++) {
			for (int z = -1; z < 2; z++) {
				wm->AddChunkToQueue(x, z);
			}
		}
		// std::cout << "Generated " << newChunks << " Chunks" << std::endl;
	}
	while (!wm->QueueIsEmpty()) {
		// Wait for chunks to finish loading
	}
	// TODO: Wait for queue to finish
	Int3 spawnBlock = overworld->FindSpawnableBlock(Int3{0, 64, 0});
	auto spawnPoint = Int3ToVec3(spawnBlock);
	spawnPoint.y += STANCE_OFFSET;
	server.SetSpawnPoint(spawnPoint);

	// Create threads for sending and receiving data
	std::jthread join_thread(&Betrock::Server::ServerJoin);
	std::vector<uint8_t> response;

	while (server.IsAlive()) {
		response.clear();
		// Server is alive

		if (doDaylightCycle) {
			server.SetServerTime(server.GetServerTime() + 20);
		}
		Respond::Time(response, server.GetServerTime());
		BroadcastToPlayers(response);
		sleep(1); // Send data every second
	}

	join_thread.join();
	server.PrepareForShutdown();
	return 0;
}
