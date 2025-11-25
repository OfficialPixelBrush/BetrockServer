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

void UsageReport() {
	// Exit out if we don't want to log usage metrics
	if (!debugReportUsage) return;
	auto &server = Betrock::Server::Instance();
	std::ofstream logFile;
	logFile.open("usage.csv");
	[[maybe_unused]] int pseudoTicks = 0;
	logFile << "Ticks,Players,Chunks,Populated,Queued,Busy,Usage (MB)"<< std::endl;
	while(server.IsAlive()) {
		World *overworld = server.GetWorld(0);
		WorldManager *wm = server.GetWorldManager(0);
		std::string usage = GetUsedMemoryMBString();
		std::cout << usage << std::endl;
		logFile << pseudoTicks << ",";
		logFile << server.GetConnectedClients().size() << ",";
		if (overworld) {
			logFile << overworld->GetNumberOfChunks() << ",";
			logFile << overworld->GetNumberOfPopulatedChunks() << ",";
		} else {
			logFile << 0 << "," << 0 << ",";
		}
		if (wm) {
			logFile << wm->GetQueueSize() << ",";
			logFile << wm->GetBusyWorkers() << ",";
		} else {
			logFile << 0 << "," << 0 << ",";
		}
		logFile << usage << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000/TICK_SPEED));
		pseudoTicks++;
		//Respond::ChatMessage(response, usage);
	}
	logFile.close();
}

int main() {
	auto &server = Betrock::Server::Instance();
	auto &logger = Betrock::Logger::Instance();
	std::thread usageThread(UsageReport);

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
	auto startTime = std::chrono::steady_clock::now();
	auto lastTime = std::chrono::steady_clock::now();
	int worldIndex = 0;
	int totalQueuedChunks = 0;

	//for (int worldIndex = 0; worldIndex < worldManagers.size(); ++worldIndex) {
		logger.Info("Preparing start region for level " + std::to_string(worldIndex));

		WorldManager *wm = server.GetWorldManager(0);
		World *overworld = server.GetWorld(0);

		for (int x = -radius; x <= radius && server.IsAlive(); x += 16) {
			for (int z = -radius; z <= radius && server.IsAlive(); z += 16) {
				wm->ForceGenerateChunk(x >> 4, z >> 4);
				totalQueuedChunks++;
			}
		}

	while(!wm->IsQueueEmpty()) {
		auto now = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count();

		if (elapsed > 1000) {
			int done = totalQueuedChunks - wm->GetQueueSize();
			int percent = (done * 100) / totalQueuedChunks;
			logger.Info("Preparing spawn area: " + std::to_string(percent) + "%");
			lastTime = now;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	Int3 spawn = Int3{0, 64, 0};
	wm->FindSpawnableBlock(spawn);
	spawn.y += 3;
	server.SetSpawnPoint(spawn);
	//}

	//auto spawnPoint = Int3{0,200,0};
	//spawnPoint.y += STANCE_OFFSET;

	logger.Info("Done (" + std::to_string(
		std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::steady_clock::now() - startTime
		).count()
	) + "ms)!");


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
	usageThread.join();
	return 0;
}
