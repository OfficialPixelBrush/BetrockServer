#include "server.h"
bool alive = true;
int server_fd;

std::vector<Player*> connectedPlayers;
std::mutex connectedPlayersMutex;
std::mutex entityIdMutex;

int32_t latestEntityId;
uint64_t serverTime = 0;
int chunkDistance = 10;

std::unordered_map<int8_t, std::unique_ptr<WorldManager>> worldManagers;
std::unordered_map<int8_t, std::thread> worldManagerThreads;
Vec3 spawnPoint;
int8_t spawnWorld;

WorldManager* GetWorldManager(int8_t worldId) {
    return worldManagers[worldId].get();
}

World* GetWorld(int8_t worldId) {
    return &worldManagers[worldId]->world;
}

void AddWorldManager(int8_t worldId) {
    // Create a WorldManager and move it into the map
    auto worldManager = std::make_unique<WorldManager>();

    // Start a thread for the WorldManager's processing loop
    worldManagerThreads[worldId] = std::thread([wm = worldManager.get()]() {
        wm->Run(); // Ensure WorldManager has a `Run` function for its main loop
    });

    // Store the WorldManager in the map
    worldManagers[worldId] = std::move(worldManager);
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