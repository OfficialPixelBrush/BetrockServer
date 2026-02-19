#include "world.h"

#include "server.h"

// Returns the number of Chunks that're currently loaded into memory
int32_t World::GetNumberOfChunks() { return chunks.size(); }

int32_t World::GetNumberOfPopulatedChunks() {
	int32_t populatedChunks = 0;
	for (const auto &[key, chunkPtr] : chunks) {
		if (chunkPtr && chunkPtr->state == ChunkState::Populated) {
			populatedChunks++;
		}
	}
	return populatedChunks;
}

int32_t World::GetNumberOfModifiedChunks() {
	int32_t modifiedChunks = 0;
	for (const auto &[key, chunkPtr] : chunks) {
		if (chunkPtr && chunkPtr->modified) {
			modifiedChunks++;
		}
	}
	return modifiedChunks;
}

int8_t World::GetHeightValue(Int2 position) {
	std::shared_ptr<Chunk> c = GetChunk(
		Int2{position.x >> 4, position.y >> 4}
	);
	if (!c)
		return 0;
	return c->GetHeightValue(position.x & 15, position.y & 15);
}

// Checks if a file with a matching position and extension exists
bool World::ChunkFileExists(Int2 position, std::string extension) {
	if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
		std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << "\n";
		return false;
	}

	// Create the chunk entry file path based on x and z coordinates
	std::filesystem::path entryPath = dirPath / (std::to_string(position.x) + "," + std::to_string(position.y) + extension);

	// Check if the entry file exists and has a .cnk extension
	if (std::filesystem::exists(entryPath) && std::filesystem::is_regular_file(entryPath) &&
		entryPath.extension() == extension) {
		return true;
	}
	return false;
}

bool World::ChunkExists(Int2 position) {
	std::shared_lock lock(chunkMutex);
	return chunks.contains(GetChunkHash(position));
}

bool World::BlockExists(Int3 pos) { return pos.y >= 0 && pos.y < 128 ? ChunkExists(Int2{pos.x >> 4, pos.z >> 4}) : false; }

bool World::IsChunkGenerated(Int2 position) {
	std::shared_ptr<Chunk> c = this->GetChunk(position);
	if (!c)
		return false;
	return c->state == ChunkState::Generated;
}

// Checks if the Chunk is populated
bool World::IsChunkPopulated(Int2 position) {
	std::shared_ptr<Chunk> c = this->GetChunk(position);
	if (!c)
		return false;
	return c->state == ChunkState::Populated;
}

// Sets the directory path of the world upon creation
World::World(const std::string &extra) : dev(), rng(dev()) {
	// std::cout << "CWD: " << std::filesystem::current_path() << "\n";

	dirPath = std::filesystem::current_path() / std::string(Betrock::GlobalConfig::Instance().Get("level-name"));

	// Create dirPath first
	if (!std::filesystem::create_directories(dirPath)) {
		// std::cout << "Failed to create: " << dirPath << "\n";
	}

	// Then dimension sub-directories
	if (!extra.empty()) {
		dirPath /= extra;
		if (!std::filesystem::create_directories(dirPath)) {
			// std::cout << "Failed to create: " << dirPath << "\n";
		}
	}

	// The region folder in that
	dirPath /= "region";
	if (!std::filesystem::create_directories(dirPath)) {
		std::cout << "Failed to create: " << dirPath << "\n";
	}
}

/**
 * @brief Saves all the Chunks that're currently loaded into Memory
 */
void World::Save() {
	for (auto &pair : chunks) {
		int64_t hash = pair.first;
		std::shared_ptr<Chunk> chunk = pair.second;

		Int2 pos = DecodeChunkHash(hash);
		SaveChunk(pos, chunk);
	}
}

/**
 * @brief Gets the Chunk Pointer from Memory
 * 
 * @param x X Chunk coordinate
 * @param z Z Chunk coordinate
 */
std::shared_ptr<Chunk> World::GetChunk(Int2 position) {
	std::shared_lock lock(chunkMutex);
	auto it = chunks.find(GetChunkHash(position));
	if (it != chunks.end() && it->second != nullptr)
		return it->second;
	return nullptr;
}

/**
 * @brief Adds a new Chunk to the world
 * 
 * @param x X Chunk coordinate
 * @param z Z Chunk coordinate
 */
std::shared_ptr<Chunk> World::AddChunk(Int2 position, std::shared_ptr<Chunk> c) {
	std::unique_lock lock(chunkMutex);
	auto hash = GetChunkHash(position);
	chunks[hash] = std::move(c);
	return chunks[hash];
}

/**
 * @brief Removes a Chunk from the world
 * 
 * @param x X Chunk coordinate
 * @param z Z Chunk coordinate
 */
void World::RemoveChunk(Int2 position) {
	std::unique_lock lock(chunkMutex);
	chunks.erase(GetChunkHash(position));
}

/**
 * @brief Removes any chunks that're not visible to any player
 * 
 */
void World::FreeUnseenChunks() {
	std::vector<Int2> chunksToRemove;

	for (auto &pair : chunks) {
		const int64_t &hash = pair.first;
		std::shared_ptr<Chunk> chunk = pair.second;
		Int2 pos = DecodeChunkHash(hash);

		// Check if any player has this chunk hash in their visibleChunks
		bool isVisible = false;
		for (const auto &c : Betrock::Server::Instance().GetConnectedClients()) {
			if (c->ChunkIsVisible(pos)) {
				isVisible = true;
				break;
			}
		}

		if (!isVisible) {
			SaveChunk(pos, chunk);
			chunksToRemove.push_back(pos);
		}
	}

	// Remove chunks that are not visible to any player
	for (Int2 pos : chunksToRemove) {
		RemoveChunk(pos);
	}
}

std::shared_ptr<Chunk> World::LoadMcRegionChunk(Int2 position) {
	if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
		std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << "\n";
		return nullptr;
	}

	try {
		int32_t regionX = int32_t(floorf(position.x) / 32.0f);
		int32_t regionZ = int32_t(floorf(position.y) / 32.0f);
		std::filesystem::path regionPath =
			dirPath / ("r." + std::to_string(regionX) + "." + std::to_string(regionZ) + MCREGION_FILE_EXTENSION);
		std::cout << regionPath << "\n";

		// TODO: Keep track of which regions already exist
		std::unique_ptr<RegionFile> rf = std::make_unique<RegionFile>(regionPath);

		std::shared_ptr<NbtTag> readRoot;
		readRoot = rf->GetChunkNbt(position);

		if (!readRoot) {
			return nullptr;
		}
		std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this, position);
		c->ReadFromNbt(std::dynamic_pointer_cast<CompoundNbtTag>(readRoot));
		c->state = ChunkState::Populated;
		c->modified = false;
		return AddChunk(position, std::move(c));
	} catch (const std::exception &e) {
		Betrock::Logger::Instance().Error(e.what());
		return nullptr;
	}
}

/**
 * @brief Save a Chunk as an NBT-format file
 * 
 * @param x X Chunk coordinate
 * @param z Z Chunk coordinate
 * @param chunk The chunk that should be saved
 */
void World::SaveChunk(Int2 position, std::shared_ptr<Chunk> chunk) {
	// Skip if we have this flag set
	if (debugDisableSave)
		return;

	if (!chunk || !chunk->modified) {
		return;
	}

	// Update Chunklight before saving
	std::filesystem::path filePath =
		dirPath / (std::to_string(position.x) + "," + std::to_string(position.y) + CHUNK_FILE_EXTENSION);
	// CalculateChunkLight(GetChunk(x,z));

	std::ofstream writeFile(filePath, std::ios::binary);
	NbtWrite(writeFile, chunk->GetAsNbt(), NBT_ZLIB);
	writeFile.close();
	chunk->modified = false;
}

void World::PlaceSponge(Int3 position) {
	PlaceBlockUpdate(position, BLOCK_SPONGE);
	for (int32_t x = -2; x <= 2; x++) {
		for (int32_t z = -2; z <= 2; z++) {
			for (int32_t y = -2; y <= 2; y++) {
				BlockType blockType = GetBlockType(position + Int3{x, y, z});
				if (blockType == BLOCK_WATER_STILL || blockType == BLOCK_WATER_FLOWING) {
					PlaceBlockUpdate(position + Int3{x, y, z}, BLOCK_AIR);
				}
			}
		}
	}
}

void World::PlaceBlock(Int3 position, BlockType type, int8_t meta) { PlaceBlockUpdate(position, type, meta, false); }

// Place a block at the passed position
// This position must be within a currently loaded Chunk
void World::PlaceBlockUpdate(Int3 pos, BlockType type, int8_t meta, bool sendUpdate) {
	// Get Block Position within Chunk
	SetBlockTypeAndMeta(type, meta, pos);
	if (sendUpdate)
		UpdateBlock(pos);
}

static constexpr int32_t MAX_LIGHTING_UPDATES = 1'000'000;
static constexpr int32_t SCHEDULE_DUP_SCAN = 5;
static constexpr int32_t UPDATING_ITER_LIMIT = 500;
static constexpr int32_t MAX_CONCURRENT_SCHEDULED = 50;

// ---- SpreadLight (unchanged semantics; ensure it pushes to back) ----
void World::SpreadLight(bool skyLight, Int3 pos, int32_t newLightLevel) {
	if (!BlockExists(pos))
		return;

	int32_t oldLevel = GetLight(skyLight, pos);
	if (newLightLevel <= oldLevel)
		return; // no improvement, skip

	// Update block light in chunk if chunk is present
	if (std::shared_ptr<Chunk> c = GetChunk(Int2{pos.x >> 4, pos.z >> 4})) {
		c->SetLight(skyLight, {pos.x & 15, pos.y, pos.z & 15}, newLightLevel);
	}

	// Enqueue for further propagation (push to back to follow Java semantics of add -> remove last)
	{
		std::unique_lock<std::shared_mutex> lock(lightUpdateMutex);
		if (int32_t(lightingToUpdate.size()) >= MAX_LIGHTING_UPDATES) {
			// drop silently to avoid unbounded memory; you can log if desired
		} else {
			lightingToUpdate.emplace_back(LightUpdate{skyLight, pos, Int3{}});
		}
	}
}

void World::ScheduleLightingUpdate(bool skyLight, Int3 pos1, Int3 pos2, bool checkDuplicates) {
	// worldProvider.hasNoSky()
	if (true && skyLight)
		return;

	++lightingUpdatesScheduled;
	try {
		if (lightingUpdatesScheduled >= MAX_CONCURRENT_SCHEDULED) {
			--lightingUpdatesScheduled;
			return;
		}

		int32_t midX = (pos1.x + pos2.x) / 2;
		int32_t midZ = (pos1.z + pos2.z) / 2;

		if (!BlockExists({midX, 64, midZ})) {
			--lightingUpdatesScheduled;
			return;
		}

		std::shared_ptr<Chunk> chunk = GetChunk(Int2{midX >> 4, midZ >> 4});
		if (!chunk) {
			--lightingUpdatesScheduled;
			return;
		}

		// Duplicate suppression: scan up to last SCHEDULE_DUP_SCAN entries
		if (checkDuplicates) {
			std::unique_lock<std::shared_mutex> lock(lightUpdateMutex);
			int32_t scan = std::min(int32_t(lightingToUpdate.size()), SCHEDULE_DUP_SCAN);
			for (int32_t i = 0; i < scan; ++i) {
				LightUpdate &lu = lightingToUpdate[lightingToUpdate.size() - 1 - i];

				// Here we compare skyLight plus bounding-box overlap heuristic.
				if (lu.skyLight != skyLight)
					continue;
				if (MergeBox(lu.posA, lu.posB, pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z)) {
					return;
				}
			}
		}

		// push new scheduled update as a single-point32_t LightUpdate using posA,posB to store bbox if needed
		{
			std::unique_lock<std::shared_mutex> lock(lightUpdateMutex);
			if (int32_t(lightingToUpdate.size()) >= MAX_LIGHTING_UPDATES) {
				// too many, clear as Java did (or drop)
				lightingToUpdate.clear();
			} else {
				// store bbox endpoints in posA/posB (or adapt your LightUpdate structure)
				lightingToUpdate.emplace_back(
					LightUpdate{skyLight, Int3{pos1.x, pos1.y, pos1.z}, Int3{pos2.x, pos2.y, pos2.z}});
			}
		}
	} catch (...) {
		--lightingUpdatesScheduled;
		throw;
	}
	--lightingUpdatesScheduled;
}

bool World::UpdatingLighting() {
	// limit concurrent updating runs
	int32_t counter = lightingUpdatesCounter.load();
	if (counter >= MAX_CONCURRENT_SCHEDULED) {
		return false;
	}
	++lightingUpdatesCounter;
	try {
		int32_t iterationsLeft = UPDATING_ITER_LIMIT;

		while (true) {
			LightUpdate task;
			{
				std::unique_lock<std::shared_mutex> lock(lightUpdateMutex);
				if (lightingToUpdate.empty())
					break;
				// Java removes from end: LIFO behavior
				task = lightingToUpdate.back();
				lightingToUpdate.pop_back();
			}

			if (--iterationsLeft <= 0) {
				return true; // aborted because iteration cap reached
			}

			// Process this single LightUpdate.
			// Implement func_4127_a(this) equivalent: here we'll run localized propagation starting at task.posA.
			// Use the same propagation logic as UpdateLightingInfdev but confined per-task.
			ProcessSingleLightUpdate(task);
		}

		return false; // finished without hitting limit
	} catch (...) {
		--lightingUpdatesCounter;
		throw;
	}
	--lightingUpdatesCounter;
}

// ---- Helper: process a single LightUpdate with same 6-neighbour spread used earlier ----
void World::ProcessSingleLightUpdate(const LightUpdate &current) {
	Int3 pos = current.posA;
	if (!BlockExists(pos))
		return;

	int32_t currentLevel = GetLight(current.skyLight, pos);
	int32_t translucency = std::max<uint8_t>(1, GetOpacity(GetBlockType(pos)));

	static const Int3 dirs[6] = {{1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}};

	for (const auto &d : dirs) {
		Int3 n = {pos.x + d.x, pos.y + d.y, pos.z + d.z};
		if (!BlockExists(n))
			continue;

		int32_t neighborLevel = GetLight(current.skyLight, n);
		int32_t newLevel = std::max(0, currentLevel - translucency);

		if (newLevel > neighborLevel) {
			if (std::shared_ptr<Chunk> c = GetChunk(Int2{n.x >> 4, n.z >> 4})) {
				c->SetLight(current.skyLight, {n.x & 15, n.y, n.z & 15}, newLevel);
				// schedule further propagation by pushing new LightUpdate
				std::unique_lock<std::shared_mutex> lock(lightUpdateMutex);
				if (int32_t(lightingToUpdate.size()) < MAX_LIGHTING_UPDATES) {
					lightingToUpdate.emplace_back(LightUpdate{current.skyLight, n, Int3{}});
				}
			}
		}
	}
}

bool World::MergeBox(Int3 &posA, Int3 &posB, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6) {
	int32_t &bx1 = posA.x, &by1 = posA.y, &bz1 = posA.z;
	int32_t &bx2 = posB.x, &by2 = posB.y, &bz2 = posB.z;

	// fully inside
	if (a1 >= bx1 && a2 >= by1 && a3 >= bz1 && a4 <= bx2 && a5 <= by2 && a6 <= bz2)
		return true;

	const int32_t t = 1;
	if (a1 >= bx1 - t && a2 >= by1 - t && a3 >= bz1 - t && a4 <= bx2 + t && a5 <= by2 + t && a6 <= bz2 + t) {

		int32_t old_dx = bx2 - bx1, old_dy = by2 - by1, old_dz = bz2 - bz1;
		int32_t old_vol = old_dx * old_dy * old_dz;

		int32_t nx1 = (a1 > bx1 ? bx1 : a1);
		int32_t ny1 = (a2 > by1 ? by1 : a2);
		int32_t nz1 = (a3 > bz1 ? bz1 : a3);
		int32_t nx2 = (a4 < bx2 ? bx2 : a4);
		int32_t ny2 = (a5 < by2 ? by2 : a5);
		int32_t nz2 = (a6 < bz2 ? bz2 : a6);

		int32_t new_dx = nx2 - nx1, new_dy = ny2 - ny1, new_dz = nz2 - nz1;
		int32_t new_vol = new_dx * new_dy * new_dz;

		if (new_vol - old_vol <= 2) {
			bx1 = nx1;
			by1 = ny1;
			bz1 = nz1;
			bx2 = nx2;
			by2 = ny2;
			bz2 = nz2;
			return true;
		}
	}
	return false;
}

/**
 * @brief Inform all clients of the updated block at the passed position
 * 
 * @param position Updated block position
 */
void World::UpdateBlock(Int3 position) {
	std::vector<uint8_t> response;
	Respond::BlockChange(response, position, GetBlockType(position), GetBlockMeta(position));
	BroadcastToClients(response);
}

int8_t World::GetBlockLight(Int3 position) {
	std::shared_ptr<Chunk> c = GetChunk(Int2{position.x >> 4, position.y >> 4});
	if (!c)
		return 0;
	return c->GetBlockLight(position);
}

void World::SetBlockLight(int8_t level, Int3 position) {
	std::shared_ptr<Chunk> c = GetChunk(Int2{position.x >> 4, position.y >> 4});
	if (!c)
		return;
	c->SetBlockLight(level, position);
}

// Get the Skylight of a Block at the passed position
int8_t World::GetSkyLight(Int3 position) {
	std::shared_ptr<Chunk> c = GetChunk(Int2{position.x >> 4, position.y >> 4});
	if (!c)
		return 0;
	return c->GetSkyLight(position);
}

// Set the Skylight of a Block at the passed position
void World::SetSkyLight(int8_t level, Int3 position) {
	std::shared_ptr<Chunk> c = GetChunk(Int2{position.x >> 4, position.y >> 4});
	if (!c)
		return;
	c->SetSkyLight(level, position);
}

/**
 * @brief Get all the sign tile-entities within the requested chunk
 * 
 * @param position The requested chunk position
 * @return std::vector<SignTile *> 
 */
std::vector<SignTile *> World::GetChunkSigns(Int2 position) {
	std::shared_ptr<Chunk> c = GetChunk(Int2{position.x >> 4, position.y >> 4});
	if (!c) {
		return {};
	}
	return c->GetSigns();
}

// Get all the block,meta,block light and sky light data of a Chunk in a Binary Format
void World::GetChunkData(uint8_t *chunkData, Int2 position) {
	int32_t index = 0;
	std::shared_ptr<Chunk> c = GetChunk(position);
	if (!c)
		return;

	// BlockData
	for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
		for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
			for (uint8_t cY = 0; cY < CHUNK_HEIGHT; cY++) {
				chunkData[index] = c->GetBlockType(Int3{cX, cY, cZ});
				index++;
			}
		}
	}

	// Block Metadata
	for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
		for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
			for (uint8_t cY = 0; cY < (CHUNK_HEIGHT / 2); cY++) {
				int8_t b1 = c->GetBlockMeta(Int3{cX, cY * 2, cZ});
				int8_t b2 = c->GetBlockMeta(Int3{cX, cY * 2 + 1, cZ});
				chunkData[index] = (b2 << 4 | b1);
				index++;
			}
		}
	}

	// Block Light
	for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
		for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
			for (uint8_t cY = 0; cY < (CHUNK_HEIGHT / 2); cY++) {
				int8_t b1 = c->GetBlockLight(Int3{cX, cY * 2, cZ});
				int8_t b2 = c->GetBlockLight(Int3{cX, cY * 2 + 1, cZ});
				chunkData[index] = (b2 << 4 | b1);
				index++;
			}
		}
	}

	// Sky Light
	for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
		for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
			for (uint8_t cY = 0; cY < (CHUNK_HEIGHT / 2); cY++) {
				int8_t b1 = c->GetSkyLight(Int3{cX, cY * 2, cZ});
				int8_t b2 = c->GetSkyLight(Int3{cX, cY * 2 + 1, cZ});
				chunkData[index] = (b2 << 4 | b1);
				index++;
			}
		}
	}
}

int8_t World::GetFirstUncoveredBlock(Int3 &position) {
	for (position.y = 63; GetBlockType(Int3{position.x, position.y + 1, position.z}) != BLOCK_AIR; ++position.y) {
	}

	return GetBlockType(position);
}

// Load a Chunk into Memory from an NBT-Format file
std::shared_ptr<Chunk> World::LoadOldV2Chunk(Int2 posiiton) {
	if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
		std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << "\n";
		return nullptr;
	}

	// Create the chunk entry file path based on x and z coordinates
	std::filesystem::path entryPath = dirPath / (std::to_string(posiiton.x) + "," + std::to_string(posiiton.y) + CHUNK_FILE_EXTENSION);

	// Check if the entry file exists and has a .cnk extension
	if (!ChunkFileExists(posiiton)) {
		std::cout << "File doesn't exist!" << "\n";
		return nullptr;
	}

	try {
		std::ifstream readFile(entryPath, std::ios::binary);
		std::shared_ptr<NbtTag> readRoot = NbtRead(readFile, NBT_ZLIB, -1, CHUNK_DATA_SIZE * 2);
		readFile.close();
		if (!readRoot) {
			throw std::runtime_error("Unable to read NBT data!");
		}
		std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this, posiiton);
		c->ReadFromNbt(std::dynamic_pointer_cast<CompoundNbtTag>(readRoot));
		c->state = ChunkState::Populated;
		c->modified = false;
		return AddChunk(posiiton, std::move(c));
	} catch (const std::exception &e) {
		Betrock::Logger::Instance().Error(e.what());
		return nullptr;
	}
}

// Load an old-format Chunk into Memory from a Binary File
std::shared_ptr<Chunk> World::LoadOldChunk(Int2 posiiton) {
	if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
		std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << "\n";
		return nullptr;
	}

	// Create the chunk entry file path based on x and z coordinates
	std::filesystem::path entryPath =
		dirPath / (std::to_string(posiiton.x) + "," + std::to_string(posiiton.y) + OLD_CHUNK_FILE_EXTENSION);

	// Check if the entry file exists and has a .cnk extension
	if (!ChunkFileExists(posiiton, OLD_CHUNK_FILE_EXTENSION)) {
		return nullptr;
	}

	std::ifstream chunkFile(entryPath, std::ios::binary);
	if (!chunkFile.is_open()) {
		Betrock::Logger::Instance().Warning("Failed to load chunk " + std::string(entryPath));
		return nullptr;
	}

	// Get the length of the file
	chunkFile.seekg(0, std::ios::end);
	std::streamsize size = chunkFile.tellg();
	chunkFile.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	chunkFile.read(buffer.data(), size);
	char *compressedChunk = buffer.data();

	size_t compressedSize = size;
	size_t decompressedSize = 0;

	auto chunkData = DecompressChunk(compressedChunk, compressedSize, decompressedSize);

	if (!chunkData) {
		Betrock::Logger::Instance().Warning("Failed to decompress " + std::string(entryPath));
		return nullptr;
	}

	std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this, posiiton);
	size_t blockDataSize = CHUNK_WIDTH_X * CHUNK_WIDTH_Z * CHUNK_HEIGHT;
	size_t nibbleDataSize = CHUNK_WIDTH_X * CHUNK_WIDTH_Z * (CHUNK_HEIGHT / 2);
	for (size_t i = 0; i < decompressedSize; i++) {
		if (i < blockDataSize) {
			// Block Data
			c->SetBlockType(BlockType(chunkData[i]), BlockIndexToPosition(i));
		} else if (
			// Metadata
			i >= blockDataSize && i < blockDataSize + nibbleDataSize) {
			c->SetBlockMeta((chunkData[i]) & 0xF, BlockIndexToPosition((i % nibbleDataSize) * 2));
			c->SetBlockMeta((chunkData[i] >> 4) & 0xF, BlockIndexToPosition((i % nibbleDataSize) * 2 + 1));
		} else if (
			// Block Light
			i >= blockDataSize + nibbleDataSize && i < blockDataSize + (nibbleDataSize * 2)) {
			c->SetBlockLight((chunkData[i]) & 0xF, BlockIndexToPosition((i % nibbleDataSize) * 2));
			c->SetBlockLight((chunkData[i] >> 4) & 0xF, BlockIndexToPosition((i % nibbleDataSize) * 2 + 1));
		} else if (
			// Sky Light
			i >= blockDataSize + (nibbleDataSize * 2) && i < blockDataSize + (nibbleDataSize * 3)) {
			c->SetSkyLight((chunkData[i]) & 0xF, BlockIndexToPosition((i % nibbleDataSize) * 2));
			c->SetSkyLight((chunkData[i] >> 4) & 0xF, BlockIndexToPosition((i % nibbleDataSize) * 2 + 1));
		}
	}
	c->state = ChunkState::Populated;
	chunkFile.close();
	Betrock::Logger::Instance().Info("Updated " + std::string(entryPath));
	// Delete the old chunk file
	remove(entryPath);
	c->state = ChunkState::Populated;
	c->modified = false;
	return AddChunk(posiiton, std::move(c));
}

bool World::InteractWithBlock(Int3 pos) {
	BlockType blockType = GetBlockType(pos);
	int8_t blockMeta = GetBlockMeta(pos);
	if (blockType == BLOCK_TRAPDOOR) {
		blockMeta ^= 0b100;
	}
	if (blockType == BLOCK_DOOR_WOOD) {
		blockMeta ^= 0b100;
		Int3 nPos = pos;
		if (blockMeta & 0b1000) {
			// Interacted with Top
			// Update Bottom
			nPos = pos + Int3{0, -1, 0};
		} else {
			// Interacted with Bottom
			// Update Top
			nPos = pos + Int3{0, 1, 0};
		}
		BlockType otherBlockType = GetBlockType(nPos);
		int8_t otherBlockMeta = GetBlockMeta(nPos);
		if (otherBlockType == blockType) {
			otherBlockMeta ^= 0b100;
			SetBlockTypeAndMeta(otherBlockType, otherBlockMeta, nPos);
			UpdateBlock(nPos);
		}
	}
	SetBlockTypeAndMeta(blockType, blockMeta, pos);
	UpdateBlock(pos);
	return true;
}

// Tick all currently loaded chunks
void World::TickChunks() {
	// for (auto& pair : chunks) {
	// int64_t hash = pair.first;
	// std::shared_ptr<Chunk> chunk = pair.second.get();
	// std::uniform_int_distribution<int32_t> dist6(0,CHUNK_WIDTH_X*CHUNK_HEIGHT*CHUNK_WIDTH_Z);
	/*
	// Choose a batch of random blocks within a chunk to run RandomTick on
	for (int32_t i = 0; i < 16; i++) {
		int32_t blockIndex = dist6(rng);
		Block* b = &chunk->blocks[blockIndex];
		if (!b) continue;

		Int3 chunkPos = DecodeChunkHash(hash);
		Int3 blockPos = GetBlockPosition(blockIndex);
		Int3 pos = {
			chunkPos.x<<4 | blockPos.x,
			blockPos.y,
			chunkPos.z<<4 | blockPos.z
		};
		// If the block was changed, send this to the clients
		int8_t blockType = b->type;
		int8_t blockMeta = b->meta;
		if (!RandomTick(pos)) continue;
		if (blockType != b->type || blockMeta != b->meta) {
			UpdateBlock(pos);
		}
	}
	*/
	//}
}

// Tick the passed block
bool World::RandomTick([[maybe_unused]] Int3 &pos) {
	// Redo this, please
	return false;
}

int8_t World::GetLight(bool skyLight, Int3 pos) {
	if (pos.y < 0) {
		return 15;
	} else if (pos.y >= 128) {
		return 15;
	} else {
		std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
		if (!c)
			return 0;
		pos.x &= 15;
		pos.z &= 15;
		return c->GetLight(skyLight, pos);
	}
}

int8_t World::GetTotalLight(Int3 pos) {
	std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
	if (!c)
		return 0;
	pos.x &= 15;
	pos.z &= 15;
	return c->GetTotalLight(pos);
}

void World::SetLight(bool skyLight, Int3 pos, int8_t newLight) {
	if (pos.y < 0) {
		return;
	} else if (pos.y >= 128) {
		return;
	} else {
		std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
		if (!c)
			return;
		pos.x &= 15;
		pos.z &= 15;
		return c->SetLight(skyLight, pos, newLight);
	}
}

void World::SetBlockMeta(int8_t blockMeta, Int3 pos) {
	if (pos.y < 0) {
		return;
	} else if (pos.y >= CHUNK_HEIGHT) {
		return;
	} else {
		std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
		if (!c)
			return;
		pos.x &= 15;
		pos.z &= 15;
		return c->SetBlockMeta(blockMeta, pos);
	}
}

int8_t World::GetBlockMeta(Int3 pos) {
	if (pos.y < 0) {
		return 0;
	} else if (pos.y >= CHUNK_HEIGHT) {
		return 0;
	} else {
		std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
		if (!c)
			return 0;
		pos.x &= 15;
		pos.z &= 15;
		return c->GetBlockMeta(pos);
	}
}

void World::SetBlockType(BlockType blockType, Int3 pos) {
	if (pos.y < 0) {
		return;
	} else if (pos.y >= CHUNK_HEIGHT) {
		return;
	} else {
		std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
		if (!c)
			return;
		pos.x &= 15;
		pos.z &= 15;
		return c->SetBlockType(blockType, pos);
	}
}

BlockType World::GetBlockType(Int3 pos) {
	if (pos.y < 0) {
		return BLOCK_AIR;
	} else if (pos.y >= CHUNK_HEIGHT) {
		return BLOCK_AIR;
	} else {
		std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
		if (!c)
			return BLOCK_AIR; // TODO: Could be BLOCK_INVALID too
		pos.x &= 15;
		pos.z &= 15;
		return c->GetBlockType(pos);
	}
}

void World::SetBlockTypeAndMeta(BlockType blockType, int8_t blockMeta, Int3 pos) {
	if (pos.y < 0) {
		return;
	} else if (pos.y >= CHUNK_HEIGHT) {
		return;
	} else {
		std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
		if (!c)
			return;
		pos.x &= 15;
		pos.z &= 15;
		return c->SetBlockTypeAndMeta(blockType, blockMeta, pos);
	}
}

void World::AddTileEntity(std::unique_ptr<TileEntity> &&te) {
	std::shared_ptr<Chunk> c = this->GetChunk(Int2{te->position.x >> 4, te->position.z >> 4});
	if (!c)
		return;
	c->AddTileEntity(std::move(te));
}

TileEntity *World::GetTileEntity(Int3 pos) {
	std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
	if (!c)
		return nullptr;
	return c->GetTileEntity(pos);
}

bool World::CanBlockSeeTheSky(Int3 pos) {
	std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.z >> 4});
	if (!c)
		return false;
	return c->CanBlockSeeTheSky(pos);
}

int32_t World::GetHighestSolidOrLiquidBlock(Int2 pos) {
	std::shared_ptr<Chunk> c = this->GetChunk(Int2{pos.x >> 4, pos.y >> 4});
	if (!c)
		return -1;
	for (int32_t y = CHUNK_HEIGHT - 1; y > 0; --y) {
		BlockType blockType = this->GetBlockType(Int3{pos.x, y, pos.y});
		if (blockType == BLOCK_AIR)
			continue;
		if (IsSolid(blockType) || IsLiquid(blockType)) {
			return y + 1;
		}
	}
	return -1;
}