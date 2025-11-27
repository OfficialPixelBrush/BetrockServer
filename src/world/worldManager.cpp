#include "worldManager.h"

#include "server.h"

// This creates a new QueueChunk with a pre-filled Client
QueueChunk::QueueChunk(Int3 pPosition, const std::shared_ptr<Client> &pRequestClient) {
	this->position = pPosition;
	AddClient(pRequestClient);
}

// This adds a new Client to the Queued Chunk
void QueueChunk::AddClient(const std::shared_ptr<Client> &pRequestClient) {
	requestedClients.push_back(pRequestClient);
}

WorldManager::WorldManager(int maxThreads) {
	if (maxThreads < 1) {
		workerCount = std::thread::hardware_concurrency();
		return;
	}
	workerCount = maxThreads;
}

// This adds a Chunk to the ChunkQueue
void WorldManager::AddChunkToQueue(int32_t pX, int32_t pZ, const std::shared_ptr<Client> &pRequestClient) {
	auto hash = GetChunkHash(pX, pZ); // Compute hash

	std::lock_guard<std::mutex> lock(queueMutex); // Ensure thread safety
	if (chunkPositions.find(hash) != chunkPositions.end()) {
		// Chunk is already in the queue, no need to add it again
		for (QueueChunk &qc : chunkQueue) {
			if (qc.position.x == pX && qc.position.z == pZ) {
				qc.requestedClients.push_back(pRequestClient);
				break;
			}
		}
		return;
	}

	// Add to queue and track position
	chunkQueue.emplace_back(Int3{pX, 0, pZ}, pRequestClient);
	chunkPositions.insert(hash);
	queueCV.notify_one();
}

// Returns if the ChunkQueue is empty
bool WorldManager::IsQueueEmpty() { return chunkQueue.empty(); }

// Sets the seed of both the WorldManager and the world
void WorldManager::SetSeed(int64_t pSeed) {
	seed = pSeed;
	world.seed = pSeed;
}

// Gets the current Seed
int64_t WorldManager::GetSeed() { return seed; }

// Returns how many entries are in the Queue
int32_t WorldManager::GetQueueSize() { return chunkQueue.size(); }

int32_t WorldManager::GetBusyWorkers() { return busyWorkers.load(std::memory_order_relaxed); }

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
		// GenerateQueuedChunks();
		world.UpdatingLighting();
		std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Sleep for half a second
	}

	// Stop workers
	queueCV.notify_all();

	for (auto &worker : workers) {
		if (worker.joinable())
			worker.join();
	}
}

// This wakes up a worker thread so it starts generating a chunk
void WorldManager::GenerateQueuedChunks() {
	std::lock_guard<std::mutex> lock(queueMutex);
	// Wake up a worker thread if it's sleeping
	if (!chunkQueue.empty())
		queueCV.notify_one();
}

// Forces the generation of the passed Chunk Position,
// independent of any Worker Thread
void WorldManager::ForceGenerateChunk(int32_t x, int32_t z) { AddChunkToQueue(x, z, nullptr); }

// This is run by all the available Worker Threads
// To generate a chunk, if some are queued
void WorldManager::WorkerThread() {
	auto &cfg = Betrock::GlobalConfig::Instance();
	auto gen = cfg.Get("generator");
	// std::cout << gen << "\n";
	std::unique_ptr<Generator> generator; // = std::make_unique<GeneratorInfdev20100327>(seed, &this->world);
	// I wish C++ supported strings in switch statements
	//    generator
	if (gen == "inf20100227") {
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

			if (!Betrock::Server::Instance().IsAlive())
				return; // Exit thread when stopping

			cq = chunkQueue.front();
			chunkQueue.pop_front();
			chunkPositions.erase(GetChunkHash(cq.position.x, cq.position.z));
		}

		busyWorkers.fetch_add(1, std::memory_order_relaxed);

		std::shared_ptr<Chunk> c = GetChunk(cq.position.x, cq.position.z, generator.get());

		if (!c) {
			busyWorkers.fetch_sub(1, std::memory_order_relaxed);
			continue; // Something went wrong; skip this one
		}

		if (c->state != ChunkState::Populated) {
			std::scoped_lock lock(queueMutex);
			cq.generationAttempt++;
			// Only put a chunk back in if its not been retried too often
			if (cq.generationAttempt < MAX_GENERATION_ATTEMPTS) {
				chunkQueue.push_back(cq);
			}
			chunkPositions.insert(GetChunkHash(cq.position.x, cq.position.z));
			queueCV.notify_one();
			busyWorkers.fetch_sub(1, std::memory_order_relaxed);
			continue;
		}

		if (c) {
			std::scoped_lock lock(Betrock::Server::Instance().GetConnectedClientMutex());
			for (auto &weak : cq.requestedClients) {
				if (auto client = weak.lock()) {
					client->AddNewChunk(cq.position);
				}
			}
		}
		busyWorkers.fetch_sub(1, std::memory_order_relaxed);
		// std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Sleep for 1/10th a second
	}
}

// Match the Beta 1.7.3 Spawn block behavior
Int3 WorldManager::FindSpawnableBlock(Int3 &position) {
	JavaRandom jr;
	// auto &server = Betrock::Server::Instance();
	//  Try random offsets until we find a valid spawn coordinate
	int attempts;
	for (attempts = 0; attempts < 100 && !CanCoordinateBeSpawn(position); ++attempts) {
		position.x += jr.nextInt(64) - jr.nextInt(64);
		position.z += jr.nextInt(64) - jr.nextInt(64);
	}
	position.y = CHUNK_HEIGHT;
	world.GetFirstUncoveredBlock(position);

	return position;
}

bool WorldManager::CanCoordinateBeSpawn(Int3 &position) {
	// Generate chunk beforehand
	int8_t blockType = world.GetFirstUncoveredBlock(position);
	return blockType == BLOCK_SAND;
}

// This gets a chunk from the world if it isn't already in memory
// If a chunk file already exists for it, the file is just loaded into memory
// If the file is an old-format chunk, its loaded into memory and saved as a new-format chunk on the next save-cycle
// If the chunk doesn't yet exist on-disk, its generated by one of the worker threads.
std::shared_ptr<Chunk> WorldManager::GetChunk(int32_t cX, int32_t cZ, Generator *generator) {
	// If the chunk already exists, return right away
	std::shared_ptr<Chunk> c = world.GetChunk(cX, cZ);
	if (c)
		return c;
	// Try to load from McRegion file
	// TODO: Maybe runs out of file handles or something?
	// c = world.LoadMcRegionChunk(cX, cZ);
	// if (c) return c;
	// Old .ncnk format (NBT)
	if (world.ChunkFileExists(cX, cZ)) {
		c = world.LoadOldV2Chunk(cX, cZ);
		c->state = ChunkState::Populated;
		return c;
	}
	// Old .cnk format (Raw)
	if (world.ChunkFileExists(cX, cZ, OLD_CHUNK_FILE_EXTENSION)) {
		c = world.LoadOldChunk(cX, cZ);
		c->state = ChunkState::Populated;
		return c;
	}
	// Everything else has failed, so generate a new chunk!
	c = world.AddChunk(cX, cZ, generator->GenerateChunk(cX, cZ));

	auto tryPopulate = [&](int32_t x, int32_t z) -> bool {
		std::shared_ptr<Chunk> chunk = world.GetChunk(x, z);
		// Neighbor not loaded yet
		if (!chunk)
			return false;
		// Already populated
		if (chunk->state == ChunkState::Populated)
			return true;
		// Attempt population
		if (!generator->PopulateChunk(x, z))
			return false;
		// Chunk is marked as populated if this succeeds
		chunk->state = ChunkState::Populated;
		return true;
	};

	// Populate this chunk if neighbors exist
	if (world.ChunkExists(cX + 1, cZ + 1) && world.ChunkExists(cX, cZ + 1) && world.ChunkExists(cX + 1, cZ)) {
		tryPopulate(cX, cZ);
	}

	// Populate neighbor chunks if conditions are met
	if (world.ChunkExists(cX - 1, cZ + 1) && world.ChunkExists(cX, cZ + 1) && world.ChunkExists(cX - 1, cZ)) {
		tryPopulate(cX - 1, cZ);
	}

	if (world.ChunkExists(cX + 1, cZ - 1) && world.ChunkExists(cX, cZ - 1) && world.ChunkExists(cX + 1, cZ)) {
		tryPopulate(cX, cZ - 1);
	}

	if (world.ChunkExists(cX - 1, cZ - 1) && world.ChunkExists(cX, cZ - 1) && world.ChunkExists(cX - 1, cZ)) {
		tryPopulate(cX - 1, cZ - 1);
	}

	return c;
}

// Sets the wm name
void WorldManager::SetName(std::string pName) { this->name = pName; }

// Gets the wm name
std::string WorldManager::GetName() { return this->name; }

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

	// For ease of programming, this currently goes unused.
	// The relevant variables are stored in server.properties
	data->Put(std::make_shared<LongTag>("RandomSeed", seed));
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

	std::ofstream writeFile(levelName + "/level.dat", std::ios::binary);
	NbtWrite(writeFile, root);
	writeFile.close();
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