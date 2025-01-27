#pragma once
#include <unordered_map>
#include <vector>
#include <mutex>

#include "world.h"
#include "generator.h"
#include "coms.h"

class QueueChunk {
    public:
        Int3 position;
        std::vector<Player*> requestedPlayers;
        QueueChunk() : position(Int3()), requestedPlayers() {}
        QueueChunk(Int3 position, Player* requestPlayer = nullptr);
        void AddPlayer(Player* requestPlayer);
};

class WorldManager {
    private:
        std::string name;
        std::mutex queueMutex;
        std::unordered_map<int64_t, QueueChunk> chunkQueue;
        uint64_t seed;
    public:
        World world;
        Generator generator;
        void AddChunkToQueue(int32_t x, int32_t z, Player* requestPlayer = nullptr);
        void GenerateQueuedChunks();
        void SetSeed(int64_t seed);
        int64_t GetSeed();
        void Run();
        void CalculateColumnLight(int32_t x, int32_t z);
        void CalculateChunkLight(int32_t cX, int32_t cZ);
        void SetName(std::string name);
        std::string GetName();
        bool QueueIsEmpty();
};

std::string ConvertIndexIntoExtra(int8_t worldId);