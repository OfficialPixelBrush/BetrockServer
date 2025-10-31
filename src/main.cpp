#include <signal.h>

#include "config.h"
#include "server.h"
#include "biomes.h"
#include "gamerules.h"
#include "sysinfo.h"

// The Save interval in ticks
// This matches what Minecraft does
// 6000 = 5 minutes
// TODO: Apparently minecraft saves every 40 ticks???
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

	GenerateBiomeLookup();

	server.LoadConfig();

	auto port = Betrock::GlobalConfig::Instance().GetAsNumber<uint16_t>("server-port");
	logger.Info("Starting " + std::string(PROJECT_NAME) + " on *:" + std::to_string(port));

	if (!server.SocketBootstrap(port)) {
		return EXIT_FAILURE;
	}

	// Read in the usernames of all operators
	server.ReadOperators();
	server.ReadWhitelist();

	CommandManager::Init();
	
	// Init the plugins
	server.InitPlugins();

	// Generate spawn area
	short radius = 196;
	auto lastTime = std::chrono::steady_clock::now();
	int worldIndex = 0;

	//for (int worldIndex = 0; worldIndex < worldManagers.size(); ++worldIndex) {
		logger.Info("Preparing start region for level " + std::to_string(worldIndex));

		WorldManager *wm = server.GetWorldManager(0);
		World *overworld = server.GetWorld(0);

		for (int x = -radius; x <= radius && server.IsAlive(); x += 16) {
			for (int z = -radius; z <= radius && server.IsAlive(); z += 16) {
				auto now = std::chrono::steady_clock::now();
				auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();

				if (elapsed > 1000) {
					int total = (radius * 2 + 1) * (radius * 2 + 1);
					int done = (x + radius) * (radius * 2 + 1) + z + radius + 1;
					int percent = (done * 100) / total;
					logger.Info("Preparing spawn area: " + std::to_string(percent) + "%");
					lastTime = now;
				}

				wm->ForceGenerateChunk(x >> 4, z >> 4);

				std::this_thread::sleep_for(std::chrono::milliseconds(5));
			}
		}

	Int3 spawn = wm->FindSpawnableBlock(Int3{0, 64, 0});
	spawn.y += 2;
	server.SetSpawnPoint(spawn);
	//}

	//auto spawnPoint = Int3{0,200,0};
	//spawnPoint.y += STANCE_OFFSET;

	// Create threads for sending and receiving data
	std::thread join_thread(&Betrock::Server::ServerJoin);
	std::vector<uint8_t> response;

	int64_t lastSave = 0;
	int64_t lastTimeUpdate = 0;

	std::ofstream logFile;
	logFile.open("usage.csv");

	while (server.IsAlive()) {
		if (debugReportUsage) {
			std::string usage = GetUsedMemoryMBString();
			std::cout << usage << std::endl;
			logFile << server.GetUpTime() << ",";
			logFile << server.GetConnectedClients().size() << ",";
			logFile << overworld->GetNumberOfChunks() << ",";
			logFile << usage << std::endl;
			Respond::ChatMessage(response, usage);
		}

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
	logFile.close();

	server.Stop();
	join_thread.join();
	return 0;
}
