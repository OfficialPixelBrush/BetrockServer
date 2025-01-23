#include "server.h"
bool alive = true;
int server_fd;

std::vector<Player*> connectedPlayers;
std::mutex connectedPlayersMutex;
std::mutex entityIdMutex;

int32_t latestEntityId;
uint64_t serverTime = 0;
int chunkDistance = 4;

World overworld;
World nether;
Vec3 spawnPoint;
int8_t spawnDimension = Overworld;

World* GetDimension(int8_t dimension) {
    switch(dimension) {
        case Overworld:
            return &overworld;
        case Nether:
            return &nether;
    }
    return nullptr;
}

Player* FindPlayerByUsername(std::string username) {
    auto player = std::find_if(connectedPlayers.begin(), connectedPlayers.end(),
        [&username](const auto& player) {
            return player->username == username;
        });
    if (player != connectedPlayers.end()) {
        return *player;
    }
    return nullptr;
}