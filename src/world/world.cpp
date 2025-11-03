#include "world.h"

#include "server.h"

// Returns the number of Chunks that're currently loaded into memory
int World::GetNumberOfChunks() {
    return chunks.size();
}

int World::GetNumberOfPopulatedChunks() {
    int populatedChunks = 0;
    for (const auto& [key, chunkPtr] : chunks) {
        if (chunkPtr && chunkPtr->state == ChunkState::Populated) {
            populatedChunks++;
        }
    }
    return populatedChunks;
}


int8_t World::GetHeightValue(int32_t x, int32_t z) {
    Chunk* c = GetChunk(x >> 4, z >> 4);
    if (!c) return 0;
    return c->GetHeightValue(x & 15, z & 15);
}

// Checks if a file with a matching position and extension exists
bool World::ChunkFileExists(int32_t x, int32_t z, std::string extension) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return false;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + extension);

    // Check if the entry file exists and has a .cnk extension
    if (std::filesystem::exists(entryPath) && std::filesystem::is_regular_file(entryPath) && entryPath.extension() == extension) {
        return true;
    }
    return false;
}

bool World::ChunkExists(int32_t x, int32_t z) {
    std::shared_lock lock(chunkMutex);
    return chunks.contains(GetChunkHash(x,z));
}

bool World::BlockExists(Int3 pos) {
    return pos.y >= 0 && pos.y < 128 ? ChunkExists(pos.x >> 4, pos.z >> 4) : false;
}

bool World::IsChunkGenerated(int32_t x, int32_t z) {
    Chunk* c = this->GetChunk(x,z);
    if (!c) return false;
    return c->state == ChunkState::Generated;
}

// Checks if the Chunk is populated
bool World::IsChunkPopulated(int32_t x, int32_t z) {
    Chunk* c = this->GetChunk(x,z);
    if (!c) return false;
    return c->state == ChunkState::Populated;
}

// Sets the directory path of the world upon creation
World::World(const std::string& extra)
    : dev(), rng(dev())
{
    //std::cout << "CWD: " << std::filesystem::current_path() << "\n";

    dirPath = std::filesystem::current_path() / std::string(Betrock::GlobalConfig::Instance().Get("level-name"));

    // Create dirPath first
    if (!std::filesystem::create_directories(dirPath)) {
        //std::cout << "Failed to create: " << dirPath << std::endl;
    }

    // Then dimension sub-directories
    if (!extra.empty()) {
        dirPath /= extra;
        if (!std::filesystem::create_directories(dirPath)) {
            //std::cout << "Failed to create: " << dirPath << std::endl;
        }
    }

    // The region folder in that
    dirPath /= "region";
    if (!std::filesystem::create_directories(dirPath)) {
        std::cout << "Failed to create: " << dirPath << std::endl;
    }
}

// Saves all the Chunks that're currently loaded into Memory
void World::Save() {
    for (auto& pair : chunks) {
        int64_t hash = pair.first;
        Chunk* chunk = pair.second.get();

        Int3 pos = DecodeChunkHash(hash);
        SaveChunk(pos.x, pos.z, chunk);
    }
}

// Gets the Chunk Pointer from Memory
Chunk* World::GetChunk(int32_t x, int32_t z) {
    std::shared_lock lock(chunkMutex);
    auto it = chunks.find(GetChunkHash(x, z));
    if (it != chunks.end() && it->second != nullptr)
        return it->second.get();
    return nullptr;
}

// Adds a new Chunk to the world
Chunk* World::AddChunk(int32_t x, int32_t z, std::unique_ptr<Chunk> c) {
    std::unique_lock lock(chunkMutex);
    auto hash = GetChunkHash(x, z);
    chunks[hash] = std::move(c);
    return chunks[hash].get();
}

// Removes a Chunk from the world
void World::RemoveChunk(int32_t x, int32_t z) {
    std::unique_lock lock(chunkMutex);
    chunks.erase(GetChunkHash(x,z));
}

// Removes any chunks that're not visible to any player
void World::FreeUnseenChunks() {
    std::vector<Int3> chunksToRemove;

    for (auto& pair : chunks) {
        const int64_t& hash = pair.first;
        Chunk* chunk = pair.second.get();
        Int3 pos = DecodeChunkHash(hash);
    
        // Check if any player has this chunk hash in their visibleChunks
        bool isVisible = false;
        for (const auto& c : Betrock::Server::Instance().GetConnectedClients()) {
            if (c->ChunkIsVisible(pos)) {
                isVisible = true;
                break;
            }
        }
    
        if (!isVisible) {
            SaveChunk(pos.x, pos.z, chunk);
            chunksToRemove.push_back(pos);
        }
    }    

    // Remove chunks that are not visible to any player
    for (Int3 pos : chunksToRemove) {
        RemoveChunk(pos.x, pos.z);
    }
}

Chunk* World::LoadMcRegionChunk(int32_t cX, int32_t cZ) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return nullptr;
    }
    
    try {
        int32_t regionX = int32_t(floor(cX / 32.0f));
        int32_t regionZ = int32_t(floor(cZ / 32.0f));
        std::filesystem::path regionPath =
            dirPath /
            ("r." + std::to_string(regionX) + "." + std::to_string(regionZ) + MCREGION_FILE_EXTENSION);
        std::cout << regionPath << std::endl;
        
        // TODO: Keep track of which regions already exist
        std::unique_ptr<RegionFile> rf =
            std::make_unique<RegionFile>(regionPath);

        std::shared_ptr<Tag> readRoot;
        readRoot = rf->GetChunkNbt(cX,cZ);

        if (!readRoot) {
            return nullptr;
        }
        std::unique_ptr<Chunk> c = std::make_unique<Chunk>(this,cX,cZ);
        c->ReadFromNbt(std::dynamic_pointer_cast<CompoundTag>(readRoot));
        c->state = ChunkState::Populated;
        return AddChunk(cX,cZ,std::move(c));
    } catch (const std::exception& e) {
        Betrock::Logger::Instance().Error(e.what());
        return nullptr;
    }
}

// Save a Chunk as an NBT-format file
void World::SaveChunk(int32_t x, int32_t z, Chunk* chunk) {
    // Skip if we have this flag set
    if (debugDisableSave) return;

    if (!chunk || !chunk->modified) {
        //
    }

    // Update Chunklight before saving
    Int3 pos = Int3{x,0,z};

    std::filesystem::path filePath = dirPath / (std::to_string(pos.x) + "," + std::to_string(pos.z) + CHUNK_FILE_EXTENSION);
    //CalculateChunkLight(GetChunk(x,z));

    std::ofstream writeFile(filePath, std::ios::binary);
    NbtWrite(writeFile,chunk->GetAsNbt(),NBT_ZLIB);
    writeFile.close();
    chunk->modified = false;
}

void World::PlaceSponge(Int3 position) {
    PlaceBlockUpdate(position, BLOCK_SPONGE);
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            for (int y = -2; y <= 2; y++) {
                int8_t blockType = GetBlockType(position + Int3{x,y,z});
                if (
                    blockType == BLOCK_WATER_STILL ||
                    blockType == BLOCK_WATER_FLOWING
                ) {
                    PlaceBlockUpdate(position + Int3{x,y,z}, BLOCK_AIR);
                }
            }
        }
    }
}


void World::PlaceBlock(Int3 position, int8_t type, int8_t meta) {
    PlaceBlockUpdate(position,type,meta,false);
}

// Place a block at the passed position
// This position must be within a currently loaded Chunk
void World::PlaceBlockUpdate(Int3 pos, int8_t type, int8_t meta, bool sendUpdate) {
    // Get Block Position within Chunk
    SetBlockTypeAndMeta(type, meta, pos);
    if (sendUpdate) UpdateBlock(pos);
}

// This is just called "a" in the Source Code
void World::SpreadLight(bool skyLight, Int3 pos, int newLightLevel) {
    if (!BlockExists(pos)) return;

    int oldLevel = GetLight(skyLight, pos);
    if (newLightLevel <= oldLevel) return; // no improvement, skip

    // Update block light
    if (Chunk* c = GetChunk(pos.x >> 4, pos.z >> 4)) {
        c->SetLight(skyLight, {pos.x & 15, pos.y, pos.z & 15}, newLightLevel);
    }

    // Enqueue for further propagation
    std::unique_lock<std::shared_mutex> lock(lightUpdateMutex);
    lightingToUpdate.push(LightUpdate(skyLight, pos, Int3{}));
}

// This is just called "a" in the Source Code
void World::AddToLightQueue(bool skyLight, Int3 posA, Int3 posB) {
    std::unique_lock<std::shared_mutex> lock(lightUpdateMutex);  // Ensure thread safety
    this->lightingToUpdate.emplace(LightUpdate(skyLight,posA,posB));
}

void World::UpdateLightingInfdev() {
    std::queue<LightUpdate> queue;

    {   // transfer updates to local queue
        std::unique_lock<std::shared_mutex> lock(lightUpdateMutex);
        while (!lightingToUpdate.empty()) {
            queue.push(lightingToUpdate.front());
            lightingToUpdate.pop();
        }
    }

    while (!queue.empty()) {
        LightUpdate current = queue.front();
        queue.pop();

        Int3 pos = current.posA;
        if (!BlockExists(pos)) continue;

        int currentLevel = GetLight(current.skyLight, pos);
        int translucency = std::max(uint8_t(1), GetTranslucency(GetBlockType(pos)));

        // Spread to 6 neighbors
        static const Int3 dirs[6] = {
            { 1, 0, 0}, {-1, 0, 0},
            { 0, 1, 0}, { 0,-1, 0},
            { 0, 0, 1}, { 0, 0,-1}
        };

        for (const auto& d : dirs) {
            Int3 n = { pos.x + d.x, pos.y + d.y, pos.z + d.z };
            if (!BlockExists(n)) continue;

            int neighborLevel = GetLight(current.skyLight, n);
            int newLevel = std::max(0, currentLevel - translucency);

            if (newLevel > neighborLevel) {
                if (Chunk* c = GetChunk(n.x >> 4, n.z >> 4)) {
                    c->SetLight(current.skyLight, {n.x & 15, n.y, n.z & 15}, newLevel);
                    queue.push(LightUpdate(current.skyLight, n, Int3{}));
                }
            }
        }
    }
}

void World::UpdateBlock(Int3 position) {
    std::vector<uint8_t> response;
    Respond::BlockChange(
        response,
        position,
        GetBlockType(position),
        GetBlockMeta(position)
    );
    BroadcastToClients(response);
}

int8_t World::GetBlockLight(Int3 position) {
    Chunk* c = GetChunk(position.x,position.z);
    if (!c) return 0;
    return c->GetBlockLight(position);
}

void World::SetBlockLight(int8_t level, Int3 position) {
    Chunk* c = GetChunk(position.x,position.z);
    if (!c) return;
    c->SetBlockLight(level, position);
}

// Get the Skylight of a Block at the passed position
int8_t World::GetSkyLight(Int3 position) {
    Chunk* c = GetChunk(position.x,position.z);
    if (!c) return 0;
    return c->GetSkyLight(position);
}

// Set the Skylight of a Block at the passed position
void World::SetSkyLight(int8_t level, Int3 position) {
    Chunk* c = GetChunk(position.x,position.z);
    if (!c) return;
    c->SetSkyLight(level, position);
}

std::vector<SignTile*> World::GetChunkSigns(Int3 position) {
    Chunk* c = GetChunk(position.x,position.z);
    if (!c) {
        return {};
    }
    return c->GetSigns();
}

// Get all the block,meta,block light and sky light data of a Chunk in a Binary Format
std::unique_ptr<char[]> World::GetChunkData(Int3 position) {
    auto bytes = std::make_unique<char[]>(CHUNK_DATA_SIZE);
    int index = 0;
    Chunk* c = GetChunk(position.x,position.z);
    if (!c) {
        return nullptr;
    }
    // TODO: Make use of the individual functions!!!
    // BlockData
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < CHUNK_HEIGHT; cY++) {
                bytes[index] = c->GetBlockType(Int3{cX,cY,cZ});
                index++;
            }
        }
    }

    // Block Metadata
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block* b1 = c->GetBlock(cX,cY*2,cZ);
                Block* b2 = c->GetBlock(cX,cY*2+1,cZ);
                if (!b1 || !b2) continue;
                bytes[index] = (b2->meta << 4 | b1->meta);
                index++;
            }
        }
    }

    // Block Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                int8_t b1 = c->GetBlockLight(Int3{cX,cY*2  ,cZ});
                int8_t b2 = c->GetBlockLight(Int3{cX,cY*2+1,cZ});
                bytes[index] = (b2 << 4 | b1);
                index++;
            }
        }
    }

    // Sky Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                int8_t b1 = c->GetSkyLight(Int3{cX,cY*2  ,cZ});
                int8_t b2 = c->GetSkyLight(Int3{cX,cY*2+1,cZ});
                bytes[index] = (b2 << 4 | b1);
                index++;
            }
        }
    }
    return bytes;
}

int8_t World::GetFirstUncoveredBlock(Int3& position) {
    for(
        position.y = 63;
        GetBlockType(Int3{position.x, position.y + 1, position.z}) != BLOCK_AIR;
        ++position.y
    ) {
    }

    return GetBlockType(position);
}

// Load a Chunk into Memory from an NBT-Format file
Chunk* World::LoadOldV2Chunk(int32_t x, int32_t z) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return nullptr;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + CHUNK_FILE_EXTENSION);

    // Check if the entry file exists and has a .cnk extension
    if (!ChunkFileExists(x,z)) {
        std::cout << "File doesn't exist!" << std::endl;
        return nullptr;
    }

    try {
        std::ifstream readFile(entryPath, std::ios::binary);
        std::shared_ptr<Tag> readRoot = NbtRead(readFile,NBT_ZLIB,-1,CHUNK_DATA_SIZE*2);
        readFile.close();
        if (!readRoot) {
            throw std::runtime_error("Unable to read NBT data!");
        }
        std::unique_ptr<Chunk> c = std::make_unique<Chunk>(this,x,z);
        c->ReadFromNbt(std::dynamic_pointer_cast<CompoundTag>(readRoot));
        c->state = ChunkState::Populated;
        return AddChunk(x,z,std::move(c));
    } catch (const std::exception& e) {
        Betrock::Logger::Instance().Error(e.what());
        return nullptr;
    }
}

// Load an old-format Chunk into Memory from a Binary File
Chunk* World::LoadOldChunk(int32_t x, int32_t z) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return nullptr;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + OLD_CHUNK_FILE_EXTENSION);

    // Check if the entry file exists and has a .cnk extension
    if (!ChunkFileExists(x,z,OLD_CHUNK_FILE_EXTENSION)) {
        return nullptr;
    }

    std::ifstream chunkFile (entryPath, std::ios::binary);
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
    char* compressedChunk = buffer.data();

    size_t compressedSize = size;
    size_t decompressedSize = 0;

    auto chunkData = DecompressChunk(compressedChunk,compressedSize,decompressedSize);

    if (!chunkData) {
        Betrock::Logger::Instance().Warning("Failed to decompress " + std::string(entryPath));
        return nullptr;
    }

    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(this,x,z);
    size_t blockDataSize = CHUNK_WIDTH_X*CHUNK_WIDTH_Z*CHUNK_HEIGHT;
    size_t nibbleDataSize = CHUNK_WIDTH_X*CHUNK_WIDTH_Z*(CHUNK_HEIGHT/2);
    for (size_t i = 0; i < decompressedSize; i++) {
        if (i < blockDataSize) {
            // Block Data
            c->blocks[i].type = chunkData[i];
        } else if (
            // Metadata
            i >= blockDataSize &&
            i <  blockDataSize+nibbleDataSize)
        {
            c->blocks[(i%nibbleDataSize)*2  ].meta = (chunkData[i]     )&0xF;
            c->blocks[(i%nibbleDataSize)*2+1].meta = (chunkData[i] >> 4)&0xF;
        } else if (
            // Block Light
            i >= blockDataSize+nibbleDataSize &&
            i <  blockDataSize+(nibbleDataSize*2))
        {
            SetBlockLight((chunkData[i]     )&0xF, BlockIndexToPosition((i%nibbleDataSize)*2  ));
            SetBlockLight((chunkData[i] >> 4)&0xF, BlockIndexToPosition((i%nibbleDataSize)*2+1));
        } else if (
            // Sky Light
            i >= blockDataSize+(nibbleDataSize*2) &&
            i <  blockDataSize+(nibbleDataSize*3))
        {
            SetSkyLight((chunkData[i]     )&0xF, BlockIndexToPosition((i%nibbleDataSize)*2  ));
            SetSkyLight((chunkData[i] >> 4)&0xF, BlockIndexToPosition((i%nibbleDataSize)*2+1));
        }
    }
    c->state = ChunkState::Populated;
    chunkFile.close();
    Betrock::Logger::Instance().Info("Updated " + std::string(entryPath));
    // Delete the old chunk file
    remove(entryPath);
    c->state = ChunkState::Populated;
    return AddChunk(x,z,std::move(c));
}

bool World::InteractWithBlock(Int3 pos) {
    int8_t blockType = GetBlockType(pos);
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
            nPos = pos + Int3{0,-1,0};
        } else {
            // Interacted with Bottom
            // Update Top
            nPos = pos + Int3{0,1,0};
        }
        int8_t otherBlockType = GetBlockType(nPos);
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
    for (auto& pair : chunks) {
        int64_t hash = pair.first;
        Chunk* chunk = pair.second.get();
        std::uniform_int_distribution<int32_t> dist6(0,CHUNK_WIDTH_X*CHUNK_HEIGHT*CHUNK_WIDTH_Z);
        // Choose a batch of random blocks within a chunk to run RandomTick on
        for (int i = 0; i < 16; i++) {
            int blockIndex = dist6(rng);
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
    }
}

// Tick the passed block
bool World::RandomTick([[maybe_unused]] Int3& pos) {
    // Redo this, please
    return false;
}

int8_t World::GetLight(bool skyLight, Int3 pos) {
    if(pos.y < 0) {
        return 15;
    } else if(pos.y >= 128) {
        return 15;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return 0;
        pos.x &= 15;
        pos.z &= 15;
        return c->GetLight(skyLight, pos);
    }
}

int8_t World::GetTotalLight(Int3 pos) {
    Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
    if (!c) return 0;
    pos.x &= 15;
    pos.z &= 15;
    return c->GetTotalLight(pos);
}

void World::SetLight(bool skyLight, Int3 pos, int8_t newLight) {
    if(pos.y < 0) {
        return;
    } else if(pos.y >= 128) {
        return;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return;
        pos.x &= 15;
        pos.z &= 15;
        return c->SetLight(skyLight, pos, newLight);
    }
}

void World::SetBlockMeta(int8_t blockMeta, Int3 pos) {
    if(pos.y < 0) {
        return;
    } else if(pos.y >= CHUNK_HEIGHT) {
        return;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return;
        pos.x &= 15;
        pos.z &= 15;
        return c->SetBlockMeta(blockMeta, pos);
    }
}

int8_t World::GetBlockMeta(Int3 pos) {
    if(pos.y < 0) {
        return 0;
    } else if(pos.y >= CHUNK_HEIGHT) {
        return 0;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return 0;
        pos.x &= 15;
        pos.z &= 15;
        return c->GetBlockMeta(pos);
    }
}

void World::SetBlockType(int8_t blockType, Int3 pos) {
    if(pos.y < 0) {
        return;
    } else if(pos.y >= CHUNK_HEIGHT) {
        return;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return;
        pos.x &= 15;
        pos.z &= 15;
        return c->SetBlockType(blockType, pos);
    }
}

int8_t World::GetBlockType(Int3 pos) {
    if(pos.y < 0) {
        return BLOCK_AIR;
    } else if(pos.y >= CHUNK_HEIGHT) {
        return BLOCK_AIR;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return 0;
        pos.x &= 15;
        pos.z &= 15;
        return c->GetBlockType(pos);
    }
}

void World::SetBlockTypeAndMeta(int8_t blockType, int8_t blockMeta, Int3 pos) {
    if(pos.y < 0) {
        return;
    } else if(pos.y >= CHUNK_HEIGHT) {
        return;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return;
        pos.x &= 15;
        pos.z &= 15;
        return c->SetBlockTypeAndMeta(blockType, blockMeta, pos);
    }
}

void World::AddTileEntity(std::unique_ptr<TileEntity>&& te) {
    Chunk* c = this->GetChunk(
        te->position.x >> 4,
        te->position.z >> 4
    );
    if (!c) return;
    c->AddTileEntity(std::move(te));
}

TileEntity* World::GetTileEntity(Int3 pos) {
    Chunk* c = this->GetChunk(
        pos.x >> 4,
        pos.z >> 4
    );
    if (!c) return nullptr;
    return c->GetTileEntity(pos);
}

bool World::CanBlockSeeTheSky(Int3 pos) {
    Chunk* c = this->GetChunk(
        pos.x >> 4,
        pos.z >> 4
    );
    if (!c) return false;
    return c->CanBlockSeeTheSky(pos);
}

int World::GetHighestSolidOrLiquidBlock(int32_t x, int32_t z) {
    Chunk* c = this->GetChunk(
        x >> 4,
        z >> 4
    );
    if (!c) return -1;
    for (int y = CHUNK_HEIGHT -1; y > 0; --y) {
        int8_t blockType = this->GetBlockType(Int3{x,y,z});
        if (blockType == BLOCK_AIR) continue;
        if (IsSolid(blockType) || IsLiquid(blockType)) {
            return y+1;
        }
    }
    return -1;
}