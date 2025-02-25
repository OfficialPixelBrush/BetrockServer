#include "server.h"

#include <random>
#include <ranges>

namespace Betrock {

void Server::Stop() noexcept { this->alive = false; }

bool Server::IsAlive() const noexcept { return this->alive; }

int8_t Server::GetSpawnWorld() const noexcept { return this->spawnWorld; }

int Server::GetServerFd() const noexcept { return this->serverFd; }

std::vector<Player *> &Server::GetConnectedPlayers() noexcept { return this->connectedPlayers; }

int32_t &Server::GetLatestEntityId() noexcept { return this->latestEntityId; }

int Server::GetChunkDistance() const noexcept { return this->chunkDistance; }

uint64_t Server::GetServerTime() const noexcept { return this->serverTime; }

WorldManagerMap &Server::GetWorldManagers() noexcept { return this->worldManagers; }

WorldManager *Server::GetWorldManager(int8_t worldId) const {
	if (worldManagers.contains(worldId) == false) {
		return nullptr;
	}

	return this->worldManagers.at(worldId).get();
}

World *Server::GetWorld(int8_t worldId) const {
	if (worldManagers.contains(worldId) == false) {
		return nullptr;
	}

	return &this->worldManagers.at(worldId)->world;
}

const Vec3 &Server::GetSpawnPoint() const noexcept { return this->spawnPoint; }

std::mutex &Server::GetConnectedPlayerMutex() noexcept { return this->connectedPlayersMutex; }

std::mutex &Server::GetEntityIdMutex() noexcept { return this->entityIdMutex; }

void Server::SetServerTime(uint64_t serverTime) { this->serverTime = serverTime; }

void Server::SetSpawnPoint(const Vec3 &spawnPoint) noexcept { this->spawnPoint = spawnPoint; }

Player *Server::FindPlayerByUsername(std::string_view username) const {
	auto player = std::ranges::find_if(std::ranges::views::all(this->connectedPlayers),
									   [&username](const auto &p) { return p->username == username; });

	if (player == this->connectedPlayers.end()) {
		return nullptr;
	}

	return std::to_address(*player);
}

void Server::AddWorldManager(int8_t worldId) {
	const auto &[wmEntry, wmWorked] = this->worldManagers.try_emplace(worldId, std::make_unique<WorldManager>());

	if (wmWorked == false) {
		// TODO: better error handling + logger macros
		std::cerr << "world_manager emplace failed\n";
		return;
	}

	// wm_entry is the key:value pair, so second is a reference to the unique
	// pointer.
	auto *world_manager = wmEntry->second.get();

	const auto [threadEntry, threadWorked] =
		this->worldManagerThreads.try_emplace(worldId, &WorldManager::Run, world_manager);
	if (threadWorked == false) {
		// TODO: better error handling + logger macros
		std::cerr << "thread emplace failed\n";
		return;
	}
}

void Server::SaveAll() {
	Betrock::Logger::Instance().Info("Saving...");
	for (Player* p : GetConnectedPlayers()){
		if (p) {
			Disconnect(p,"Goodbye!");
		}
	}
	for (const auto &[key, wm] : worldManagers) {
		wm->world.Save();
	}
	Betrock::Logger::Instance().Info("Saved");
}

void Server::FreeAll() {
	Betrock::Logger::Instance().Info("Freeing Chunks");
	for (const auto &[key, wm] : worldManagers) {
		wm->world.FreeUnseenChunks();
	}
}

void Server::PrepareForShutdown() {
	alive = false;
	// Save all active worlds
	if (!debugDisableSaveLoad) {
		SaveAll();
	}
	//DisconnectAllPlayers("Server closed!");
	close(serverFd);
}

void Server::LoadConfig() {
	int64_t seed = 0;

	if (!std::filesystem::exists(GlobalConfig::Instance().GetPath())) {
		GlobalConfig::Instance().Overwrite({{"level-name", "world"},
											{"view-distance", "5"},
											// {"white-list","false"},
											{"server-ip", ""},
											//{"pvp","true"},
											// use a random device to seed another prng that gives us our seed
											{"level-seed", std::to_string(std::mt19937(std::random_device()())())},
											//{"spawn-animals",true}
											{"server-port", "25565"},
											//{"allow-nether",true},
											//{"spawn-monsters","true"},
											//{"max-players","20"},
											//{"online-mode","false"},
											//{"allow-flight","false"}
											{"generator", "terrain/worley.lua"}});
		GlobalConfig::Instance().SaveToDisk();
	} else {
		GlobalConfig::Instance().LoadFromDisk();
		chunkDistance = GlobalConfig::Instance().GetAsNumber<int>("view-distance");
		seed = GlobalConfig::Instance().GetAsNumber<int>("level-seed");
	}

	// Load all defined worlds
	// TODO: Add file to configure custom worlds
	AddWorldManager(0);
	for (const auto &[key, wm] : worldManagers) {
		wm->SetSeed(seed);
		// TODO: Currently the extra part is not passed to the world!!
		//wm->SetExtra(ConvertIndexIntoExtra(key));
	}

	GlobalConfig::Instance().SaveToDisk();
}

void Server::InitPlugins() {
    // Go through /scripts/plugins folder
    // Load plugin file and let it sit in it's own lua VM
    // Start Plugin script
    for (const auto & entry : std::filesystem::directory_iterator("scripts/plugins/")) {
		try {
        	plugins.push_back(std::make_unique<Plugin>(entry.path()));
		} catch (const std::exception& e) {
        	Betrock::Logger::Instance().Error(e.what());
		}
    }
}

bool Server::SocketBootstrap(uint16_t port) {
	// TODO: remove perror in future with cooler betrock custom error stuff
	struct sockaddr_in address;

	// Create socket
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd < 0) {
		perror("Socket creation failed");
		return false;
	}

	if (int opt = 1; setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		perror("setsockopt failed");
		return false;
	}

	// Bind socket to port 25565
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("Bind failed");
		return false;
	}

	// Listen for connections
	if (listen(serverFd, 3) < 0) {
		perror("Listen failed");
		return false;
	}

	return true;
}

} // namespace Betrock
