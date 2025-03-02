#include "worldManager.h"

#include "server.h"

QueueChunk::QueueChunk(Int3 position, Client* requestClient) {
    this->position = position;
    AddClient(requestClient);
}

void QueueChunk::AddClient(Client* requestClient) {
    requestedClients.push_back(requestClient);
}

void WorldManager::AddChunkToQueue(int32_t x, int32_t z, Client* requestClient) {
    std::lock_guard<std::mutex> lock(queueMutex);  // Ensure thread safety

    auto hash = GetChunkHash(x, z);  // Compute hash

    if (chunkPositions.find(hash) != chunkPositions.end()) {
        // Chunk is already in the queue, no need to add it again
        // TODO: Add other requestClient to chunk!!
        return;
    }

    // Add to queue and track position
    chunkQueue.emplace(Int3{x, 0, z}, requestClient);
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

    // TODO: Add clean-up thread to remove unseen chunks
    while (Betrock::Server::Instance().IsAlive()) {
        GenerateQueuedChunks();
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for half a second
    }

    // Stop workers
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

    while (Betrock::Server::Instance().IsAlive()) {
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

        std::scoped_lock lock(Betrock::Server::Instance().GetConnectedClientMutex());
        for (auto c : cq.requestedClients) {
            if (c) {
                std::lock_guard<std::mutex> lock(c->GetNewChunksMutex());
                c->AddNewChunk(cq.position);
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

void WorldManager::FreeUnseenChunks() {
    world.FreeUnseenChunks();
}

void WorldManager::SaveNbt() {
	auto &server = Betrock::Server::Instance();
	auto root = std::make_shared<CompoundTag>("");
	auto data = std::make_shared<CompoundTag>("Data");
	root->Put(data);

    Int3 spawn = Vec3ToInt3(server.GetSpawnPoint());

	data->Put(std::make_shared<LongTag>("RandomSeed",seed));
	data->Put(std::make_shared<IntTag>("SpawnY", spawn.y));
	data->Put(std::make_shared<IntTag>("rainTime", 87264));
	data->Put(std::make_shared<IntTag>("thunderTime", 26271));
	data->Put(std::make_shared<IntTag>("SpawnZ", spawn.z));
	data->Put(std::make_shared<IntTag>("SpawnX", spawn.x));
	data->Put(std::make_shared<ByteTag>("raining", 0));
	data->Put(std::make_shared<LongTag>("Time", server.GetServerTime()));
	data->Put(std::make_shared<ByteTag>("thundering", 0));
	data->Put(std::make_shared<IntTag>("version", 19132));
	data->Put(std::make_shared<LongTag>("LastPlayed", 1740410572431));
    std::string levelName = std::string(Betrock::GlobalConfig::Instance().Get("level-name"));
	data->Put(std::make_shared<StringTag>("LevelName", levelName));
	data->Put(std::make_shared<LongTag>("SizeOnDisk", 3956736));

	NbtWriteToFile(levelName + "/level.dat",root);
}

/*
void WorldManager::LoadNbt() {
	auto root = std::make_shared<CompoundTag>("");
	auto data = std::make_shared<CompoundTag>("Data");
	root->Put(data);

	data->Put(std::make_shared<LongTag>("RandomSeed",8703966663084738725));
	data->Put(std::make_shared<IntTag>("SpawnY", 64));
	data->Put(std::make_shared<IntTag>("rainTime", 87264));
	data->Put(std::make_shared<IntTag>("thunderTime", 26271));
	data->Put(std::make_shared<IntTag>("SpawnZ", -51));
	data->Put(std::make_shared<IntTag>("SpawnX", 63));
	data->Put(std::make_shared<ByteTag>("raining", 0));
	data->Put(std::make_shared<LongTag>("Time", 56719));
	data->Put(std::make_shared<ByteTag>("thundering", 0));
	data->Put(std::make_shared<IntTag>("version", 19132));
	data->Put(std::make_shared<LongTag>("LastPlayed", 1740410572431));
	data->Put(std::make_shared<StringTag>("LevelName", "world"));
	data->Put(std::make_shared<LongTag>("SizeOnDisk", 3956736));
}*/