#pragma once
#include <cstdint>
#include <mutex>
#include <stdatomic.h>
#include <thread>
#include <unordered_map>
#include <vector>
#include <stdexcept>

#include "client.h"
#include "world.h"
#include "worldManager.h"
#include "plugins.h"

#define PROTOCOL_VERSION 14
#define TICK_SPEED 20
#define OPERATOR_FILE "ops.txt"

namespace Betrock {

using WorldManagerMap = std::unordered_map<int8_t, std::unique_ptr<WorldManager>>;

// tcp betrock server singleton
class Server {
  public:
	static Server &Instance() {
		// this will create the server instance just once and just return a
		// reference to the object instead of creating a new one
		static Server instance;
		return instance;
	}

	void Stop() noexcept;

	bool IsAlive() const noexcept;

	int8_t GetSpawnDimension() const noexcept;

	std::string GetSpawnWorld() const noexcept;

	int GetServerFd() const noexcept;

	std::vector<std::shared_ptr<Client>> &GetConnectedClients() noexcept;

	int32_t &GetLatestEntityId() noexcept;

	int GetChunkDistance() const noexcept;

	uint64_t GetServerTime() const noexcept;

	uint64_t GetUpTime() const noexcept;

	WorldManagerMap &GetWorldManagers() noexcept;

	// get the world manager for the world with the coresponding world_id.
	// !! returns a valid pointer or a nullptr on failure !!
	WorldManager *GetWorldManager(int8_t world_id) const;

	// This is used for managing operators
	void ReadOperators();
	void WriteOperators();
	bool AddOperator(std::string username);
	bool IsOperator(std::string username);
	bool RemoveOperator(std::string username);

	// get the world with the coresponding world_id.
	// !! returns a valid pointer or a nullptr on failure !!
	World *GetWorld(int8_t world_id) const;

	const Vec3 &GetSpawnPoint() const noexcept;

	std::mutex &GetConnectedClientMutex() noexcept;

	std::mutex &GetEntityIdMutex() noexcept;

	void SetServerTime(uint64_t serverTime);

	void AddUpTime(uint64_t upTime);

	void SetSpawnPoint(const Vec3 &spawnPoint) noexcept;

	Client *FindClientByUsername(std::string_view username) const;

	// add a new world manager (and also a new world)
	void AddWorldManager(int8_t world_id);

	void SaveAll();

	void FreeAll();

	void PrepareForShutdown();

	void LoadConfig();

	bool SocketBootstrap(uint16_t port);

	// Plugin handling
	void InitPlugins();

	static void ServerJoin() {
		auto &server = Server::Instance();
		struct sockaddr_in address;
		int addrlen = sizeof(address);

		while (server.IsAlive()) {
			// Accept connections
			int client_fd = accept(server.serverFd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
			if (client_fd < 0) {
				if (!server.IsAlive()) break;
				perror("Accept failed");
				continue;
			}

			// Create new Client
			{
				std::scoped_lock lockEntityId(server.entityIdMutex);
				auto client = std::make_shared<Client>(client_fd);
				client->SetConnectionStatus(ConnectionStatus::Handshake);

				// Add this new client to the list of connected Players
				{
					std::scoped_lock lockConnectedClients(server.connectedClientsMutex);
					server.connectedClients.push_back(client);
				}
				std::jthread clientThread(&Client::HandleClient, client);
				clientThread.detach();
			}
		}
	}

  	private:
	Server() = default;
	~Server() = default;

	// =====================================================
	// delete copy and move constructor and assignment operator to be sure no one
	// can create a second server instance
	// (deleting is basically making them unavailable)

	Server(const Server &) = delete;
	Server(const Server &&) = delete;

	Server &operator=(const Server &) = delete;
	Server &operator=(const Server &&) = delete;

	// =====================================================

	bool alive = true; // server alive
	int serverFd = -1;
	std::vector<std::shared_ptr<Client>> connectedClients;
	int32_t latestEntityId = 0;
	int chunkDistance = 10;
	atomic_uint64_t serverTime = 0;
	atomic_uint64_t upTime = 0;
	WorldManagerMap worldManagers;
	std::unordered_map<int8_t, std::jthread> worldManagerThreads;
	std::vector<std::unique_ptr<Plugin>> plugins;
	Vec3 spawnPoint;
	std::int8_t spawnDimension;
	std::string spawnWorld;
	std::vector<std::string> operators;

	std::mutex connectedClientsMutex;
	std::mutex entityIdMutex;
};
} // namespace Betrock