#pragma once
#include <queue>
#include <unordered_set>
#include <vector>
#include <mutex>
#include <condition_variable>

#include "world.h"
#include "generator.h"
#include "historic/inf20100227/generatorInfdev20100227.h"
#include "historic/inf20100327/generatorInfdev20100327.h"
#include "historic/b173/generatorBeta173.h"
#include "generatorLua.h"
#include "coms.h"
#include "client.h"
#include "lighting.h"

class Client;  // Forward declaration

class QueueChunk {
    public:
        Int3 position;
        std::vector<std::weak_ptr<Client>> requestedClients;
        QueueChunk() : position(Int3()), requestedClients() {}
        QueueChunk(Int3 position, const std::shared_ptr<Client>& requestClient = nullptr);
        void AddClient(const std::shared_ptr<Client>& requestClient);
};

class WorldManager {
    private:
        std::string name;
        std::mutex queueMutex;
        std::deque<QueueChunk> chunkQueue;
        std::unordered_set<int64_t> chunkPositions;  // Set to track chunk hashes
        int64_t seed;
        std::condition_variable queueCV;
        std::vector<std::thread> workers;
        const int workerCount = std::thread::hardware_concurrency();  // Use number of CPU cores
        void WorkerThread();
        Chunk* GetChunk(int32_t cX, int32_t cZ, Generator* generator);
    public:
        World world;
        void AddChunkToQueue(int32_t x, int32_t z, const std::shared_ptr<Client>& requestClient = nullptr);
        void GenerateQueuedChunks();
        void ForceGenerateChunk(int32_t x, int32_t z);
        void SetSeed(int64_t seed);
        int64_t GetSeed();
        void Run();
        void SetName(std::string name);
        std::string GetName();
        bool IsQueueEmpty();
        int QueueSize();
        void SaveNbt();
        void LoadNbt();
        void FreeAndSave();
};

std::string ConvertIndexIntoExtra(int8_t worldId);