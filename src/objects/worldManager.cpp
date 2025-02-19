#include "worldManager.h"

#include "server.h"

QueueChunk::QueueChunk(Int3 position, Player* requestPlayer) {
    this->position = position;
    AddPlayer(requestPlayer);
}

void QueueChunk::AddPlayer(Player* requestPlayer) {
    requestedPlayers.push_back(requestPlayer);
}

void WorldManager::AddChunkToQueue(int32_t x, int32_t z, Player* requestPlayer) {
    std::lock_guard<std::mutex> lock(queueMutex);  // Ensure thread safety

    auto hash = GetChunkHash(x, z);  // Compute hash

    if (chunkPositions.find(hash) != chunkPositions.end()) {
        // Chunk is already in the queue, no need to add it again
        return;
    }

    // Add to queue and track position
    chunkQueue.emplace(Int3{x, 0, z}, requestPlayer);
    chunkPositions.insert(hash);
}

bool WorldManager::QueueIsEmpty() {
    return chunkQueue.empty();
}

void WorldManager::SetSeed(int64_t seed) {
    this->seed = seed;
    world.seed = seed;
}

int64_t WorldManager::GetSeed() {
    return this->seed;
}

void WorldManager::Run() {
    // Start worker threads
    for (int i = 0; i < workerCount; ++i) {
        workers.emplace_back(&WorldManager::WorkerThread, this);
    }

    while (Betrock::Server::Instance().IsAlive()) {
        GenerateQueuedChunks();
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for half a second
    }

    // Stop workers
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        Betrock::Server::Instance().Stop();
    }
    queueCV.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable()) worker.join();
    }
}

void WorldManager::GenerateQueuedChunks() {
    std::lock_guard<std::mutex> lock(queueMutex);
    queueCV.notify_one();  // Wake up a worker thread if it's sleeping
}

void WorldManager::ForceGenerateChunk(int32_t x, int32_t z) {
    Generator generator;
    generator.PrepareGenerator(seed);
    GetChunk(x,z,generator);
}

void WorldManager::WorkerThread() {
    Generator generator;
    generator.PrepareGenerator(seed);

    while (true) {
        QueueChunk cq;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [this] { return !chunkQueue.empty() || !Betrock::Server::Instance().IsAlive(); });

            if (!Betrock::Server::Instance().IsAlive() && chunkQueue.empty()) return; // Exit thread when stopping

            cq = chunkQueue.front();
            chunkQueue.pop();
        }

        int64_t key = GetChunkHash(cq.position.x, cq.position.z);
        chunkPositions.erase(key);  // Remove from tracking set


		// Try to load chunk
        GetChunk(cq.position.x, cq.position.z,generator);

        std::scoped_lock lock(Betrock::Server::Instance().GetConnectedPlayerMutex());
        for (Player* p : cq.requestedPlayers) {
            if (p) {
                std::lock_guard<std::mutex> lock(p->newChunksMutex);
                p->newChunks.push_back(cq.position);
            }
        }
    }
}

void WorldManager::GetChunk(int32_t x, int32_t z, Generator &generator) {
    if (world.ChunkFileExists(x,z)) {
        world.LoadChunk(x,z);
    }  else {
        Chunk c = generator.GenerateChunk(x,z);
        world.AddChunk(x, z, c);
    }
}

void WorldManager::SetName(std::string name) {
    this->name = name;
}

std::string WorldManager::GetName() {
    return this->name;
}

std::string ConvertIndexIntoExtra(int8_t worldId) {
    if (worldId == 0) {
        return "";
    }
    return "DIM" + std::to_string(worldId);
}