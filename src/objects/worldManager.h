#pragma once
#include <queue>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <condition_variable>

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
        std::queue<QueueChunk> chunkQueue;
        std::unordered_set<int64_t> chunkPositions;  // Set to track chunk hashes
        int64_t seed;
        std::condition_variable queueCV;
        std::vector<std::thread> workers;
        const int workerCount = std::thread::hardware_concurrency();  // Use number of CPU cores
        void WorkerThread();
        void GetChunk(int32_t x, int32_t z, Generator &generator);
    public:
        World world;
        void AddChunkToQueue(int32_t x, int32_t z, Player* requestPlayer = nullptr);
        void GenerateQueuedChunks();
        void ForceGenerateChunk(int32_t x, int32_t z);
        void SetSeed(int64_t seed);
        int64_t GetSeed();
        void Run();
        void CalculateColumnLight(int32_t x, int32_t z);
        void CalculateChunkLight(int32_t cX, int32_t cZ);
        void SetName(std::string name);
        std::string GetName();
        bool QueueIsEmpty();
        void SaveNbt();
        void LoadNbt();
        void FreeUnseenChunks();
};

std::string ConvertIndexIntoExtra(int8_t worldId);