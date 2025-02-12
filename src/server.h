#pragma once
#include <cstdint>
#include <mutex>
#include <stdatomic.h>
#include <thread>
#include <unordered_map>
#include <vector>

#include "client.h"
#include "player.h"
#include "world.h"
#include "worldManager.h"
#include "plugins.h"

#define PROTOCOL_VERSION 14

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

	int8_t GetSpawnWorld() const noexcept;

	int GetServerFd() const noexcept;

	std::vector<Player *> &GetConnectedPlayers() noexcept;

	int32_t &GetLatestEntityId() noexcept;

	int GetChunkDistance() const noexcept;

	uint64_t GetServerTime() const noexcept;

	WorldManagerMap &GetWorldManagers() noexcept;

	// get the world manager for the world with the coresponding world_id.
	// !! returns a valid pointer or a nullptr on failure !!
	WorldManager *GetWorldManager(int8_t world_id) const;

	// get the world with the coresponding world_id.
	// !! returns a valid pointer or a nullptr on failure !!
	World *GetWorld(int8_t world_id) const;

	const Vec3 &GetSpawnPoint() const noexcept;

	std::mutex &GetConnectedPlayerMutex() noexcept;

	std::mutex &GetEntityIdMutex() noexcept;

	void SetServerTime(uint64_t serverTime);

	void SetSpawnPoint(const Vec3 &spawnPoint) noexcept;

	Player *FindPlayerByUsername(std::string_view username) const;

	// add a new world manager (and also a new world)
	void AddWorldManager(int8_t world_id);

	void PrepareForShutdown();

	void LoadConfig();

	bool SocketBootstrap(uint16_t port);

	// Plugin handling
	void InitPlugins();

	static void ServerJoin() {
		auto &server = Server::Instance();
		// TODO: add sockets to epoll
		struct sockaddr_in address;
		std::vector<std::jthread> playerThreadPool;
		int addrlen = sizeof(address);
		int client_fd;

		while (server.alive) {
			// Accept connections
			client_fd = accept(server.serverFd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
			if (client_fd < 0) {
				perror("Accept failed");
				continue;
			}

			// Create new player
			std::scoped_lock lockEntityId(server.entityIdMutex);

			// TODO: oh you know exactly what to do
			Player *player = new Player(client_fd, server.latestEntityId, server.spawnPoint, server.spawnWorld,
										server.spawnPoint, server.spawnWorld);

			player->connectionStatus = ConnectionStatus::Handshake;

			// Add this new player to the list of connected Players
			std::scoped_lock lockConnectedPlayers(server.connectedPlayersMutex);
			server.connectedPlayers.push_back(player);

			// Let each player have their own thread
			// TODO: Make this non-cancerous, and close player threads upon disconnect
			// IDEA: disconnect player socket in their destructor, when we try to read from a closed socket epoll will
			// yell at us
			playerThreadPool.emplace_back(HandleClient, player);
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
	std::int8_t spawnWorld;
	int serverFd = -1;
	std::vector<Player *> connectedPlayers;
	int32_t latestEntityId = 0;
	int chunkDistance = 10;
	atomic_uint64_t serverTime = 0;
	WorldManagerMap worldManagers;
	std::unordered_map<int8_t, std::jthread> worldManagerThreads;
	std::vector<Plugin> plugins;
	Vec3 spawnPoint;

	std::mutex connectedPlayersMutex;
	std::mutex entityIdMutex;
};
} // namespace Betrock