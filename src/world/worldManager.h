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

class Client;  // Forward declaration

#define MAX_GENERATION_ATTEMPTS 5

class QueueChunk {
    public:
        Int3 position;
        std::vector<std::weak_ptr<Client>> requestedClients;
        int generationAttempt = 0;
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
        int workerCount = 1;  // Use number of CPU cores
        std::atomic<int> busyWorkers = 0;
        void WorkerThread();
        std::shared_ptr<Chunk> GetChunk(int32_t cX, int32_t cZ, Generator* generator);
    public:
        WorldManager(int maxThreads = -1);
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
        void SaveNbt();
        void LoadNbt();
        void FreeAndSave();
        Int3 FindSpawnableBlock(Int3& position);
        bool CanCoordinateBeSpawn(Int3& position);
        int32_t GetQueueSize();
        int32_t GetBusyWorkers();
};

std::string ConvertIndexIntoExtra(int8_t worldId);