#pragma once
#include <vector>
#include <mutex>
#include <cstdint>
#include <algorithm>

#include "player.h"
#include "world.h"

#define PROTOCOL_VERSION 14

extern bool alive;
extern int server_fd;
extern std::vector<Player*> connectedPlayers;
extern std::mutex connectedPlayersMutex;
extern std::mutex entityIdMutex;

extern int32_t latestEntityId;
extern uint64_t serverTime;
extern int chunkDistance;

extern World overworld;
extern World nether;
extern Vec3 spawnPoint;
extern int8_t spawnDimension;

enum Dimension {
    Nether = -1,
    Overworld = 0
};

World* GetDimension(int8_t dimension);
Player* FindPlayerByUsername(std::string username);