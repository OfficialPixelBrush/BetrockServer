#include "server.h"

#include <random>
#include <ranges>

namespace Betrock {

bool Server::IsAlive() const noexcept { return this->alive; }

int8_t Server::GetSpawnDimension() const noexcept { return this->spawnDimension; }

std::string Server::GetSpawnWorld() const noexcept { return this->spawnWorld; }

int Server::GetServerFd() const noexcept { return this->serverFd; }

std::vector<std::shared_ptr<Client>> &Server::GetConnectedClients() noexcept { return this->connectedClients; }

int32_t &Server::GetLatestEntityId() noexcept { return this->latestEntityId; }

int Server::GetChunkDistance() const noexcept { return this->chunkDistance; }

uint64_t Server::GetServerTime() const noexcept { return this->serverTime; }

uint64_t Server::GetUpTime() const noexcept { return this->upTime; }

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

std::mutex &Server::GetConnectedClientMutex() noexcept { return this->connectedClientsMutex; }

std::mutex &Server::GetEntityIdMutex() noexcept { return this->entityIdMutex; }

void Server::SetServerTime(uint64_t serverTime) { this->serverTime = serverTime; }

void Server::AddUpTime(uint64_t upTime) { this->upTime += upTime; }

void Server::SetSpawnPoint(const Vec3 &spawnPoint) noexcept { this->spawnPoint = spawnPoint; }

Client *Server::FindClientByUsername(std::string_view username) const {
	auto client = std::ranges::find_if(std::ranges::views::all(this->connectedClients),
									   [&username](const auto &c) { return c->GetPlayer()->username == username; });

	if (client == this->connectedClients.end()) {
		return nullptr;
	}

	return std::to_address(*client);
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
	for (auto c : GetConnectedClients()){
		if (c) {
			c->GetPlayer()->Save();
		}
	}
	for (const auto &[key, wm] : worldManagers) {
		wm->FreeAndSave();
	}
	Betrock::Logger::Instance().Info("Saved");
}

void Server::FreeAll() {
	Betrock::Logger::Instance().Info("Freeing Chunks");
	for (const auto &[key, wm] : worldManagers) {
		wm->world.FreeUnseenChunks();
	}
	Betrock::Logger::Instance().Info("Freed Chunks");
}

void Server::PrepareForShutdown() {
	this->alive = false;
}

void Server::Stop() noexcept {
	if (!this->alive) {
		// Save all active worlds
		for (auto client : connectedClients) {
			client->DisconnectClient("Server closed!");
		}
		SaveAll();

		shutdown(serverFd, SHUT_RDWR);
		close(serverFd);
		serverFd = -1;
	}
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

void Server::ReadOperators() { ReadGeneric(OPERATOR_TYPE); }
void Server::WriteOperators() { WriteGeneric(OPERATOR_TYPE); }
bool Server::AddOperator(std::string username) { return AddGeneric(OPERATOR_TYPE, username); }
bool Server::RemoveOperator(std::string username) { return RemoveGeneric(OPERATOR_TYPE, username); }
bool Server::IsOperator(std::string username) { return IsGeneric(OPERATOR_TYPE, username); }

void Server::ReadWhitelist() { ReadGeneric(WHITELIST_TYPE); }
void Server::WriteWhitelist() { WriteGeneric(WHITELIST_TYPE); }
bool Server::AddWhitelist(std::string username) { return AddGeneric(WHITELIST_TYPE, username); }
bool Server::RemoveWhitelist(std::string username) { return RemoveGeneric(WHITELIST_TYPE, username); }
bool Server::IsWhitelist(std::string username) { return IsGeneric(WHITELIST_TYPE, username); }

// Generic File reads, writes etc.
std::vector<std::string>& Server::GetServerVector(uint8_t type) {
	switch(type) {
		case OPERATOR_TYPE:
			return operators;
		case WHITELIST_TYPE:
			return whitelist;
		default:
			return whitelist;
	}
}

std::string Server::GetGenericFilePath(uint8_t type) {
	switch(type) {
		case OPERATOR_TYPE:
			return OPERATOR_FILE;
		case WHITELIST_TYPE:
			return WHITELIST_FILE;
		default:
			return FALLBACK_FILE;
	}
}

void Server::ReadGeneric(uint8_t type) {
	std::string path = GetGenericFilePath(type);
	std::ifstream file(path);
    if (!file) {
		std::cout << "File doesn't exist!" << std::endl;
        std::ofstream createFile(path);
        createFile.close();
        return;
    }
	for( std::string username; getline( file, username ); )
	{
		AddGeneric(type, username);
	}
	file.close();
}

void Server::WriteGeneric(uint8_t type) {
	std::ofstream file(GetGenericFilePath(type));
	auto& list = GetServerVector(type);
	for (auto entry : list) {
		file << entry << std::endl;
	}
	file.close();
}

bool Server::AddGeneric(uint8_t type, std::string username) {
	auto& list = GetServerVector(type);
	auto itr = std::find(list.begin(), list.end(), username);
	// Only add if the operator doesn't already exist
	if (itr == list.end()) {
		list.push_back(username);
		WriteGeneric(type);
		return true;
	}
	return false;
}

bool Server::RemoveGeneric(uint8_t type, std::string username) {
	auto& list = GetServerVector(type);
	auto itr = std::find(list.begin(), list.end(), username);
	// Only remove if the operator does exist
	if (itr != list.end()) {
		list.erase(itr);
		WriteGeneric(type);
		return true;
	}
	return false;
}

bool Server::IsGeneric(uint8_t type, std::string username) {
	auto& list = GetServerVector(type);
	auto itr = std::find(list.begin(), list.end(), username);
	// Only add if the passed player is an operator
	if (itr != list.end())	{
		return true;
	}
	return false;
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
