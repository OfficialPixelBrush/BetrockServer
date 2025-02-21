#include <signal.h>

#include "config.h"
#include "server.h"

void __attribute__((noreturn)) HandleSignal(int) {
	Betrock::Server::Instance().PrepareForShutdown();
	shutdown(Betrock::Server::Instance().GetServerFd(), SHUT_RDWR); // Interrupt accept
	exit(0);
}

int main() {
	auto &server = Betrock::Server::Instance();
	auto &logger = Betrock::Logger::Instance();

	signal(SIGINT, HandleSignal);  // Handle Ctrl+C
	signal(SIGTERM, HandleSignal); // Handle termination signals

	logger.Log("Starting " + std::string(PROJECT_NAME) + " version " + std::string(PROJECT_VERSION_STRING));

	server.LoadConfig();

	auto port = Betrock::GlobalConfig::Instance().GetAsNumber<uint16_t>("server-port");
	logger.Log("Starting " + std::string(PROJECT_NAME) + " on *:" + std::to_string(port));

	if (!server.SocketBootstrap(port)) {
		return EXIT_FAILURE;
	}
	
	// Init the plugins
	server.InitPlugins();

	WorldManager *wm = server.GetWorldManager(0);
	World *overworld = server.GetWorld(0);

	// Generate spawn area
	if (overworld->GetNumberOfChunks() == 0) {
		std::cout << "Generating..." << std::endl;
		for (int x = -1; x < 2; x++) {
			for (int z = -1; z < 2; z++) {
				wm->ForceGenerateChunk(x, z);
			}
		}
	}
	Int3 spawnBlock = overworld->FindSpawnableBlock(Int3{0, 128, 0});
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
