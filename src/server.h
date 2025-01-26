#pragma once
#include <vector>
#include <mutex>
#include <cstdint>
#include <algorithm>
#include <thread>
#include <unordered_map>

#include "player.h"
#include "world.h"

#define PROTOCOL_VERSION 14

class WorldManager; // Forward declaration

extern bool alive;
extern int server_fd;
extern std::vector<Player*> connectedPlayers;
extern std::mutex connectedPlayersMutex;
extern std::mutex entityIdMutex;

extern int32_t latestEntityId;
extern uint64_t serverTime;
extern int chunkDistance;

extern std::unordered_map<int8_t, std::unique_ptr<WorldManager>> worldManagers;
extern std::unordered_map<int8_t, std::thread> worldManagerThreads;
extern Vec3 spawnPoint;
extern int8_t spawnWorld;

enum Dimension {
    Nether = -1,
    Overworld = 0
};

WorldManager* GetWorldManager(int8_t worldId);
void AddWorldManager(int8_t worldId);
World* GetWorld(int8_t worldId);
Player* FindPlayerByUsername(std::string username);