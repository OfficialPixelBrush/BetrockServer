#include <signal.h>

#include "config.h"
#include "server.h"

// The Save interval in ticks
// This matches what Minecraft does
// 6000 = 5 minutes
#define SAVE_INTERVAL 6000 

void HandleGracefulSignal(int) {
	Betrock::Server::Instance().PrepareForShutdown();
}

int main() {
	auto &server = Betrock::Server::Instance();
	auto &logger = Betrock::Logger::Instance();

	signal(SIGINT, HandleGracefulSignal);  // Handle Ctrl+C
	signal(SIGTERM, HandleGracefulSignal); // Handle termination signals

	logger.Info("Starting " + std::string(PROJECT_NAME) + " version " + std::string(PROJECT_VERSION_FULL_STRING));

	server.LoadConfig();

	auto port = Betrock::GlobalConfig::Instance().GetAsNumber<uint16_t>("server-port");
	logger.Info("Starting " + std::string(PROJECT_NAME) + " on *:" + std::to_string(port));

	if (!server.SocketBootstrap(port)) {
		return EXIT_FAILURE;
	}

	// Read in the usernames of all operators
	server.ReadOperators();
	server.ReadWhitelist();
	
	// Init the plugins
	server.InitPlugins();

	WorldManager *wm = server.GetWorldManager(0);
	World *overworld = server.GetWorld(0);

	// Generate spawn area
	// TODO: Figure this shit out!!!
	int issuedChunks = 0;
	if (overworld->GetNumberOfChunks() == 0) {
		logger.Info("Preparing level \"" + std::string(Betrock::GlobalConfig::Instance().Get("level-name")) + "\"");
		//wm->ForceGenerateChunk(0, 0);
		for (int x = -2; x <= 2; x++) {
			for (int z = -2; z <= 2; z++) {
				wm->ForceGenerateChunk(x, z);
				issuedChunks++;
			}
		}
	}
	logger.Info(std::to_string(issuedChunks));
	logger.Info("Preparing start region");
	while (true) {
		// Wait for chunks to be generated
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
		logger.Info("Preparing spawn area: " + std::to_string(
			int((float(issuedChunks-wm->QueueSize())/float(issuedChunks))*100.0f)
		) + "%");
		if (wm->QueueSize() < issuedChunks/3) {
			break;
		}
	}

	Int3 spawnBlock = overworld->FindSpawnableBlock(Int3{8, 127, 8});
	auto spawnPoint = Int3ToVec3(spawnBlock);
	spawnPoint.y += STANCE_OFFSET;
	server.SetSpawnPoint(spawnPoint);

	// Create threads for sending and receiving data
	std::thread join_thread(&Betrock::Server::ServerJoin);
	std::vector<uint8_t> response;

	int64_t lastSave = 0;
	int64_t lastTimeUpdate = 0;

	while (server.IsAlive()) {
		// Server is alive
		server.AddUpTime(1);


		if (server.GetUpTime() - lastSave >= SAVE_INTERVAL) {
			server.SaveAll();
			lastSave = server.GetUpTime();
		}
		if (server.GetUpTime() - lastTimeUpdate >= TICK_SPEED) {
			if (doDaylightCycle) {
				server.SetServerTime(server.GetServerTime() + TICK_SPEED);
			}
			Respond::Time(response, server.GetServerTime());
			BroadcastToClients(response);
			lastTimeUpdate = server.GetUpTime();
		}
		//???
		overworld->TickChunks();
		response.clear();

		// Sleep for one tick
		std::this_thread::sleep_for(std::chrono::milliseconds(1000/TICK_SPEED));
	}

	server.Stop();
	join_thread.join();
	return 0;
}
