#include "worldManager.h"

#include "server.h"

// This creates a new QueueChunk with a pre-filled Client
QueueChunk::QueueChunk(Int3 position, const std::shared_ptr<Client>& requestClient) {
    this->position = position;
    AddClient(requestClient);
}

// This adds a new Client to the Queued Chunk
void QueueChunk::AddClient(const std::shared_ptr<Client>& requestClient) {
    requestedClients.push_back(requestClient);
}

// This adds a Chunk to the ChunkQueue
void WorldManager::AddChunkToQueue(int32_t x, int32_t z, const std::shared_ptr<Client>& requestClient) {
    auto hash = GetChunkHash(x, z);  // Compute hash

    std::lock_guard<std::mutex> lock(queueMutex);  // Ensure thread safety
    if (chunkPositions.find(hash) != chunkPositions.end()) {
        // Chunk is already in the queue, no need to add it again
        for (QueueChunk& qc : chunkQueue) {
            if (qc.position.x == x && qc.position.z == z) {
                qc.requestedClients.push_back(requestClient);
                break;
            }
        }
        return;
    }

    // Add to queue and track position
    chunkQueue.emplace_back(Int3{x, 0, z}, requestClient);
    chunkPositions.insert(hash);
    queueCV.notify_one();
}

// Returns if the ChunkQueue is empty
bool WorldManager::IsQueueEmpty() {
    return chunkQueue.empty();
}

// Returns how many entries are in the Queue
int WorldManager::QueueSize() {
    return chunkQueue.size();
}

// Sets the seed of both the WorldManager and the world
void WorldManager::SetSeed(int64_t seed) {
    this->seed = seed;
    world.seed = seed;
}

// Gets the current Seed
int64_t WorldManager::GetSeed() {
    return this->seed;
}

// This is run when the WorldManager is started.
// Its responsible for creating the Worker Threads and making them
// Generate Queued Chunks
void WorldManager::Run() {
    // Start worker threads
    for (int i = 0; i < workerCount; ++i) {
        workers.emplace_back(&WorldManager::WorkerThread, this);
    }

    // TODO: Add clean-up thread to remove unseen chunks
    while (Betrock::Server::Instance().IsAlive()) {
        //GenerateQueuedChunks();
        world.UpdateLighting();
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for half a second
    }

    // Stop workers
    queueCV.notify_all();

    for (auto& worker : workers) {
        if (worker.joinable()) worker.join();
    }
}

// This wakes up a worker thread so it starts generating a chunk
void WorldManager::GenerateQueuedChunks() {
    std::lock_guard<std::mutex> lock(queueMutex);
    // Wake up a worker thread if it's sleeping
    if (!chunkQueue.empty()) queueCV.notify_one();
}

// Forces the generation of the passed Chunk Position,
// independent of any Worker Thread
void WorldManager::ForceGenerateChunk(int32_t x, int32_t z) {
    AddChunkToQueue(x,z,nullptr);
}

// This is run by all the available Worker Threads
// To generate a chunk, if some are queued
void WorldManager::WorkerThread() {
    auto &cfg = Betrock::GlobalConfig::Instance();
    auto gen = cfg.Get("generator");
    //std::cout << gen << std::endl;
    std::unique_ptr<Generator> generator; // = std::make_unique<GeneratorInfdev20100327>(seed, &this->world);
    // I wish C++ supported strings in switch statements
    //    generator
    if (gen == "inf20100227" ) {
        generator = std::make_unique<GeneratorInfdev20100227>(seed, &this->world);
    } else if (gen == "inf20100327") {
        generator = std::make_unique<GeneratorInfdev20100327>(seed, &this->world);
    } else if (gen == "beta173") {
        generator = std::make_unique<GeneratorBeta173>(seed, &this->world);
    } else {
        generator = std::make_unique<GeneratorLua>(seed, &this->world);
    }

    while (Betrock::Server::Instance().IsAlive()) {
        QueueChunk cq;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCV.wait(lock, [this] { return !chunkQueue.empty() || !Betrock::Server::Instance().IsAlive(); });

            if (!Betrock::Server::Instance().IsAlive()) return; // Exit thread when stopping

            cq = chunkQueue.front();
            chunkQueue.pop_front();
            chunkPositions.erase(GetChunkHash(cq.position.x, cq.position.z));
        }


        Chunk* c = GetChunk(cq.position.x, cq.position.z, generator.get());

        if (!c) {
            continue; // Something went wrong; skip this one
        }

        if (c->state != ChunkState::Populated) {
            std::scoped_lock lock(queueMutex);
            chunkQueue.push_back(cq);
            chunkPositions.insert(GetChunkHash(cq.position.x, cq.position.z));
            queueCV.notify_one();
            continue;
        }

        if (c) {
            std::scoped_lock lock(Betrock::Server::Instance().GetConnectedClientMutex());
            for (auto& weak : cq.requestedClients) {
                if (auto client = weak.lock()) {
                    client->AddNewChunk(cq.position);
                }
            }
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Sleep for 1/10th a second
    }
}

// This gets a chunk from the world if it isn't already in memory
// If a chunk file already exists for it, the file is just loaded into memory
// If the file is an old-format chunk, its loaded into memory and saved as a new-format chunk on the next save-cycle
// If the chunk doesn't yet exist on-disk, its generated by one of the worker threads.
Chunk* WorldManager::GetChunk(int32_t x, int32_t z, Generator* generator) {
    // ChunkExists is a function, but if we're gonna be using the chunk anyways, may as well do this
    Chunk* c = world.GetChunk(x,z);

    // If the chunk already exists, return right away
    if (!c) {
        if (world.ChunkFileExists(x,z)) {
            // Chunk exists as a file
            c = world.LoadChunk(x,z);
            // TODO: Populate unpopulated chunks!!!!
            c->state = ChunkState::Populated;
        } else if (world.ChunkFileExists(x,z,OLD_CHUNK_FILE_EXTENSION)) {
            // Chunk exists as a file (old format)
            c = world.LoadOldChunk(x,z);
        } else {
            c = world.AddChunk(x, z, generator->GenerateChunk(x,z));

            // TODO: Entities are loaded here

            if(world.ChunkExists(x + 1, z + 1) && world.ChunkExists(x, z + 1) && world.ChunkExists(x + 1, z)) {
                generator->PopulateChunk(x, z);
            }

            if(world.ChunkExists(x - 1, z + 1) && world.ChunkExists(x, z + 1) && world.ChunkExists(x - 1, z)) {
                generator->PopulateChunk(x - 1, z);
            }

            if(world.ChunkExists(x + 1, z - 1) && world.ChunkExists(x, z - 1) && world.ChunkExists(x + 1, z)) {
                generator->PopulateChunk(x, z - 1);
            }

            if(world.ChunkExists(x - 1, z - 1) && world.ChunkExists(x, z - 1) && world.ChunkExists(x - 1, z)) {
                generator->PopulateChunk(x - 1, z - 1);
            }
            
            c->state = ChunkState::Populated;
        }
    }
    return c;
}

// Sets the wm name
void WorldManager::SetName(std::string name) {
    this->name = name;
}

// Gets the wm name
std::string WorldManager::GetName() {
    return this->name;
}

// Used for creating the Minecraft Dimension Number, since the nether is -1, for example.
std::string ConvertIndexIntoExtra(int8_t worldId) {
    if (worldId == 0) {
        return "";
    }
    return "DIM" + std::to_string(worldId);
}

// This frees all unseen chunks from memory and saves all remaining chunks
void WorldManager::FreeAndSave() {
    // Remove all we can't see
    world.FreeUnseenChunks();
    // Save all that's left
    world.Save();
}

// Saves the world metadata to an NBT-format file
void WorldManager::SaveNbt() {
	auto &server = Betrock::Server::Instance();
	auto root = std::make_shared<CompoundTag>("");
	auto data = std::make_shared<CompoundTag>("Data");
	root->Put(data);

    Int3 spawn = server.GetSpawnPoint();

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