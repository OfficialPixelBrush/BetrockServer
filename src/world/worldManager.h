#pragma once
#include <condition_variable>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <vector>

#include "client.h"
#include "coms.h"
#include "generator.h"
#include "generatorLua.h"
#include "historic/b173/generatorBeta173.h"
#include "historic/inf20100227/generatorInfdev20100227.h"
#include "historic/inf20100327/generatorInfdev20100327.h"
#include "world.h"

class Client; // Forward declaration

#define MAX_GENERATION_ATTEMPTS 5

class QueueChunk {
  public:
	Int2 position;
	std::vector<std::weak_ptr<Client>> requestedClients;
	int generationAttempt = 0;
	QueueChunk() : position(Int2()), requestedClients() {}
	QueueChunk(Int2 position, const std::shared_ptr<Client> &requestClient = nullptr);
	void AddClient(const std::shared_ptr<Client> &requestClient);
};

class WorldManager {
  private:
	std::string name;
	std::mutex queueMutex;
	std::deque<QueueChunk> chunkQueue;
	std::unordered_set<int64_t> chunkPositions; // Set to track chunk hashes
	int64_t seed;
	std::condition_variable queueCV;
	std::vector<std::thread> workers;
	int workerCount = 1; // Use number of CPU cores
	std::atomic<int> busyWorkers = 0;
	void WorkerThread();
	std::shared_ptr<Chunk> GetChunk(Int2 position, Generator *generator);

  public:
	WorldManager(int maxThreads = -1);
	World world;
	void AddChunkToQueue(Int2 position, const std::shared_ptr<Client> &requestClient = nullptr);
	void GenerateQueuedChunks();
	void ForceGenerateChunk(Int2 position);
	void SetSeed(int64_t seed);
	int64_t GetSeed();
	void Run();
	void SetName(std::string name);
	std::string GetName();
	bool IsQueueEmpty();
	void SaveNbt();
	void LoadNbt();
	void FreeAndSave();
	Int3 FindSpawnableBlock(Int3 &position);
	bool CanCoordinateBeSpawn(Int3 &position);
	int32_t GetQueueSize();
	int32_t GetBusyWorkers();
};

std::string ConvertIndexIntoExtra(int8_t worldId);