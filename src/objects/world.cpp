#include "world.h"

#include "server.h"

// Returns the number of Chunks that're currently loaded into memory
int World::GetNumberOfChunks() {
    return chunks.size();
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

// Checks if the Chunk exists in memory
bool World::ChunkExists(int32_t x, int32_t z) {
    return chunks.contains(GetChunkHash(x,z));
}

// Sets the directory path of the world upon creation
World::World(const std::string& extra)
    : dev(), rng(dev())
{
    dirPath = Betrock::GlobalConfig::Instance().Get("level-name");
    if (extra.empty()) {
        dirPath += "/region";
    } else {
        dirPath += "/" + extra + "/region";
    }

    if (std::filesystem::create_directories(dirPath)) {
        std::cout << "Directory created: " << dirPath << '\n';
    }
}

// Saves all the Chunks that're currently loaded into Memory
void World::Save() {
    for (auto& pair : chunks) {
        const int64_t& hash = pair.first;
        Chunk& chunk = pair.second;
    
        Int3 pos = DecodeChunkHash(hash);
        SaveChunk(pos.x, pos.z, &chunk);
    }
}

// Gets the Chunk Pointer from Memory
Chunk* World::GetChunk(int32_t x, int32_t z) {
    auto it = chunks.find(GetChunkHash(x, z));
    if (it != chunks.end()) {
        return &it->second; // Return a pointer to the found chunk
    }
    return nullptr; // Return nullptr if no valid object is found
}

// Adds a new Chunk to the world
void World::AddChunk(int32_t x, int32_t z, Chunk c) {
    chunks[GetChunkHash(x,z)] = c;
}

// Removes a Chunk from the world
void World::RemoveChunk(int32_t x, int32_t z) {
    chunks.erase(GetChunkHash(x,z));
}

// Removes any chunks that're not visible to any player
void World::FreeUnseenChunks() {
    std::vector<Int3> chunksToRemove;

    for (auto& pair : chunks) {
        const int64_t& hash = pair.first;
        Chunk& chunk = pair.second;
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
            SaveChunk(pos.x, pos.z, &chunk);
            chunksToRemove.push_back(pos);
        }
    }    

    // Remove chunks that are not visible to any player
    for (Int3 pos : chunksToRemove) {
        RemoveChunk(pos.x, pos.z);
    }
}

// Load a Chunk into Memory from an NBT-Format file
bool World::LoadChunk(int32_t x, int32_t z) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return false;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + CHUNK_FILE_EXTENSION);

    // Check if the entry file exists and has a .cnk extension
    if (!ChunkFileExists(x,z)) {
        std::cout << "File doesn't exist!" << std::endl;
        return false;
    }

    try {
        // TODO: This estimate is probably overkill
        auto readRoot = NbtReadFromFile(entryPath,NBT_ZLIB,-1,CHUNK_DATA_SIZE*2);
        if (!readRoot) {
            throw std::runtime_error("Unable to read NBT data!");
        }

        auto root = std::dynamic_pointer_cast<CompoundTag>(readRoot);
        auto level = std::dynamic_pointer_cast<CompoundTag>(root->Get("Level"));
        auto blockTag = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("Blocks"));
        auto blocks = blockTag->GetData();
        auto meta = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("Data"))->GetData();
        auto blockLight = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("BlockLight"))->GetData();
        auto skyLight = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("SkyLight"))->GetData();

        auto terrainPopulated = std::dynamic_pointer_cast<ByteTag>(level->Get("TerrainPopulated"))->GetData();

        Chunk c;
        size_t blockDataSize  = (CHUNK_WIDTH_X * CHUNK_WIDTH_Z *  CHUNK_HEIGHT   );
        size_t nibbleDataSize = (CHUNK_WIDTH_X * CHUNK_WIDTH_Z * (CHUNK_HEIGHT/2));
        // Block Data
        for (size_t i = 0; i < blockDataSize; i++) {
            c.blocks[i].type = blocks[i];
        }
        // Block Metadata
        for (size_t i = 0; i < nibbleDataSize; i++) {
            c.blocks[i*2  ].meta = (meta[i]     )&0xF;
            c.blocks[i*2+1].meta = (meta[i] >> 4)&0xF;
        }
        // Block Light
        for (size_t i = 0; i < nibbleDataSize; i++) {
            c.blocks[i*2  ].lightBlock = (blockLight[i]     )&0xF;
            c.blocks[i*2+1].lightBlock = (blockLight[i] >> 4)&0xF;
        }
        // Sky Light
        for (size_t i = 0; i < nibbleDataSize; i++) {
            c.blocks[i*2  ].lightSky = (skyLight[i]     )&0xF;
            c.blocks[i*2+1].lightSky = (skyLight[i] >> 4)&0xF;
        }
        c.populated = (bool)terrainPopulated;
        AddChunk(x,z,c);
        return true;
    } catch (const std::exception& e) {
        Betrock::Logger::Instance().Error(e.what());
        return false;
    }
}

// Save a Chunk as an NBT-format file
void World::SaveChunk(int32_t x, int32_t z, Chunk* chunk) {
    if (!chunk || !chunk->modified) {
        //
    }
    // Update Chunklight before saving
    CalculateChunkLight(GetChunk(x,z));
    Int3 pos = Int3{x,0,z};

    std::filesystem::path filePath = dirPath / (std::to_string(pos.x) + "," + std::to_string(pos.z) + CHUNK_FILE_EXTENSION);

    // Acquire existing chunk data
    auto blocks = GetChunkBlocks(chunk);
    auto meta = GetChunkMeta(chunk);
    auto blockLight = GetChunkBlockLight(chunk);
    auto skyLight = GetChunkSkyLight(chunk);

    auto root = std::make_shared<CompoundTag>("");
    auto level = std::make_shared<CompoundTag>("Level");
    root->Put(level);
    level->Put(std::make_shared<ByteArrayTag>("Blocks", blocks));
    level->Put(std::make_shared<ByteArrayTag>("Data", meta));
    level->Put(std::make_shared<ByteArrayTag>("BlockLight", blockLight));
    level->Put(std::make_shared<ByteArrayTag>("SkyLight", skyLight));
    level->Put(std::make_shared<ByteTag>("TerrainPopulated", chunk->populated));
    level->Put(std::make_shared<IntTag>("zPos",x));
    level->Put(std::make_shared<IntTag>("xPos",z));
    
    NbtWriteToFile(filePath,root,NBT_ZLIB);
    chunk->modified = false;
}

// Place a block at the passed position
// This position must be within a currently loaded Chunk
void World::PlaceBlock(Int3 position, int8_t type, int8_t meta) {
    // Get Block Position within Chunk
    Block* b = GetBlock(position);
    if (!b) {
        return;
    }
    b->type = type;
    b->meta = meta;
    b->lightBlock = GetEmissiveness(b->type);
    // This needs to be recalculated
    b->lightSky = (IsTranslucent(b->type) || IsTransparent(b->type))*0xF;
    /*
    if (IsEmissive(b->type)) {
        PropagateLight(this,position,b->lightBlock,true);
    }
    */
    UpdateBlock(position,b);
}

// Remove the block and turn it into air
Block* World::BreakBlock(Int3 position) {
    // Break Block Position within Chunk
    Block* b = GetBlock(position);
    if (!b) {
        return nullptr;
    }
    b->type = 0;
    b->meta = 0;
    UpdateBlock(position,b);
    return b;
}

void World::UpdateBlock(Int3 position, Block* b) {
    if (!b) {
        return;
    }
    std::vector<uint8_t> response;
    Respond::BlockChange(response,position,b->type,b->meta);
    BroadcastToClients(response);
}

// Get the Block at the passed position
Block* World::GetBlock(Int3 position) {
    // Get Block Position within Chunk
    int32_t cX = position.x >> 4;
    int32_t cZ = position.z >> 4;
    int8_t bX = position.x & 0xF;
    int8_t bZ = position.z & 0xF;
    Chunk* c = GetChunk(cX,cZ);
    if (!c) {
        return nullptr;
    }
    c->modified = true;
    return &c->blocks[GetBlockIndex(Int3{bX,(int8_t)position.y,bZ})];
}

// Get the Skylight of a Block at the passed position
int8_t World::GetSkyLight(Int3 position) {
    // Get Block Position within Chunk
    Block* b = GetBlock(position);
    if (b) {
        return b->lightSky;
    }
    return 0;
}

// Set the Skylight of a Block at the passed position
void World::SetSkyLight(Int3 position, int8_t level) {
    // Get Block Position within Chunk
    Block* b = GetBlock(position);
    if (b) {
        b->lightSky = level;
    }
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
    for (int cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int cY = 0; cY < CHUNK_HEIGHT; cY++) {
                Block b = c->blocks[GetBlockIndex(Int3{cX,cY,cZ})];
                bytes[index] = b.type;
                index++;
            }
        }
    }

    // Block Metadata
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(Int3{cX,cY*2  ,cZ})];
                Block b2 = c->blocks[GetBlockIndex(Int3{cX,cY*2+1,cZ})];
                bytes[index] = (b2.meta << 4 | b1.meta);
                index++;
            }
        }
    }

    // Block Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(Int3{cX,cY*2  ,cZ})];
                Block b2 = c->blocks[GetBlockIndex(Int3{cX,cY*2+1,cZ})];
                bytes[index] = (b2.lightBlock << 4 | b1.lightBlock);
                index++;
            }
        }
    }

    // Sky Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(Int3{cX,cY*2  ,cZ})];
                Block b2 = c->blocks[GetBlockIndex(Int3{cX,cY*2+1,cZ})];
                bytes[index] = (b2.lightSky << 4 | b1.lightSky);
                index++;
            }
        }
    }
    return bytes;
}

// Get all the Block Data of a Chunk as an array
std::array<int8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> World::GetChunkBlocks(const Chunk* c) {
    std::array<int8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> data;
    if (!c) {
        return data;
    }
    int index = 0;
    // BlockData
    for (int cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int cY = 0; cY < CHUNK_HEIGHT; cY++) {
                Block b = c->blocks[GetBlockIndex(Int3{cX,cY,cZ})];
                data[index] = b.type;
                index++;
            }
        }
    }
    return data;
}

// Get all the Meta Data of a Chunk as an array
std::array<int8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> World::GetChunkMeta(const Chunk* c) {
    std::array<int8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> data;
    if (!c) {
        return data;
    }
    int index = 0;
    // Block Metadata
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(Int3{cX,cY*2  ,cZ})];
                Block b2 = c->blocks[GetBlockIndex(Int3{cX,cY*2+1,cZ})];
                data[index] = (b2.meta << 4 | b1.meta);
                index++;
            }
        }
    }
    return data;
}

// Get all the Block Light Data of a Chunk as an array
std::array<int8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> World::GetChunkBlockLight(const Chunk* c) {
    std::array<int8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> data;
    if (!c) {
        return data;
    }
    int index = 0;
    // Block Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(Int3{cX,cY*2  ,cZ})];
                Block b2 = c->blocks[GetBlockIndex(Int3{cX,cY*2+1,cZ})];
                data[index] = (b2.lightBlock << 4 | b1.lightBlock);
                index++;
            }
        }
    }
    return data;
}

// Get all the Sky Light Data of a Chunk as an array
std::array<int8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> World::GetChunkSkyLight(const Chunk* c) {
    std::array<int8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> data;
    if (!c) {
        return data;
    }
    int index = 0;

    // Sky Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(Int3{cX,cY*2  ,cZ})];
                Block b2 = c->blocks[GetBlockIndex(Int3{cX,cY*2+1,cZ})];
                data[index] = (b2.lightSky << 4 | b1.lightSky);
                index++;
            }
        }
    }
    return data;
}

// Find the highest possible non-solid block that can see the sky
Int3 World::FindSpawnableBlock(Int3 position) {
    bool skyVisible = true;
    Int3 spawn = position;

    for (int y = CHUNK_HEIGHT-1; y >= 0; --y) {
        spawn.y = y;
        int16_t type = GetBlock(spawn)->type;
        if (type != BLOCK_AIR) {
            // The position above this block is air
            return spawn;
        }
    }

    std::cout << "Found no suitable place to spawn, defaulting." << std::endl;
    return position;
}

// Load an old-format Chunk into Memory from a Binary File
bool World::LoadOldChunk(int32_t x, int32_t z) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return false;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + OLD_CHUNK_FILE_EXTENSION);

    // Check if the entry file exists and has a .cnk extension
    if (!ChunkFileExists(x,z,OLD_CHUNK_FILE_EXTENSION)) {
        return false;
    }

    std::ifstream chunkFile (entryPath);
    if (!chunkFile.is_open()) {
        Betrock::Logger::Instance().Warning("Failed to load chunk " + std::string(entryPath));
        return false;
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
        return false;
    }

    Chunk c;
    size_t blockDataSize = CHUNK_WIDTH_X*CHUNK_WIDTH_Z*CHUNK_HEIGHT;
    size_t nibbleDataSize = CHUNK_WIDTH_X*CHUNK_WIDTH_Z*(CHUNK_HEIGHT/2);
    for (size_t i = 0; i < decompressedSize; i++) {
        if (i < blockDataSize) {
            // Block Data
            c.blocks[i].type = chunkData[i];
        } else if (
            // Metadata
            i >= blockDataSize &&
            i <  blockDataSize+nibbleDataSize)
        {
            c.blocks[(i%nibbleDataSize)*2  ].meta = (chunkData[i]     )&0xF;
            c.blocks[(i%nibbleDataSize)*2+1].meta = (chunkData[i] >> 4)&0xF;
        } else if (
            // Block Light
            i >= blockDataSize+nibbleDataSize &&
            i <  blockDataSize+(nibbleDataSize*2))
        {
            c.blocks[(i%nibbleDataSize)*2  ].lightBlock = (chunkData[i]     )&0xF;
            c.blocks[(i%nibbleDataSize)*2+1].lightBlock = (chunkData[i] >> 4)&0xF;
        } else if (
            // Sky Light
            i >= blockDataSize+(nibbleDataSize*2) &&
            i <  blockDataSize+(nibbleDataSize*3))
        {
            c.blocks[(i%nibbleDataSize)*2  ].lightSky = (chunkData[i]     )&0xF;
            c.blocks[(i%nibbleDataSize)*2+1].lightSky = (chunkData[i] >> 4)&0xF;
        }
    }
    AddChunk(x,z,c);
    chunkFile.close();
    Betrock::Logger::Instance().Info("Updated " + std::string(entryPath));
    // Delete the old chunk file
    remove(entryPath);
    return true;
}

bool World::InteractWithBlock(Int3 pos) {
    Block* b = GetBlock(pos);
    if (!b) {
        return false;
    }
    if (b->type == BLOCK_TRAPDOOR) {
        b->meta ^= 0b100;
    }
    if (b->type == BLOCK_DOOR_WOOD) {
        b->meta ^= 0b100;
        Int3 nPos = pos;
        if (b->meta & 0b1000) {
            // Interacted with Top
            // Update Bottom
            nPos = pos + Int3{0,-1,0};
        } else {
            // Interacted with Bottom
            // Update Top
            nPos = pos + Int3{0,1,0};
        }
        Block* bb = GetBlock(nPos);
        if (bb && bb->type==b->type) {
            bb->meta ^= 0b100;
            UpdateBlock(nPos,bb);
        }
    }
    UpdateBlock(pos,b);
    return true;
}

// Tick all currently loaded chunks
void World::TickChunks() {
    for (auto& pair : chunks) {
        int64_t hash = pair.first;
        Chunk& chunk = pair.second;
        std::uniform_int_distribution<int32_t> dist6(0,CHUNK_WIDTH_X*CHUNK_HEIGHT*CHUNK_WIDTH_Z);
        // Choose a batch of random blocks within a chunk to run RandomTick on
        for (int i = 0; i < 16; i++) {
            int blockIndex = dist6(rng);
            Block* b = &chunk.blocks[blockIndex];
            int8_t oldType = b->type;
            int8_t oldMeta = b->meta;

            Int3 chunkPos = DecodeChunkHash(hash);
            Int3 blockPos = GetBlockPosition(blockIndex);
            Int3 pos = {
                chunkPos.x<<4 | blockPos.x,
                blockPos.y,
                chunkPos.z<<4 | blockPos.z
            };
            // If the block was changed, send this to the clients
            if (RandomTick(b,pos)) {
                Block* nb = GetBlock(pos);
                if (nb) {
                    UpdateBlock(pos,nb);
                    //std::cout << pos << std::endl;
                }
            }
        }
    }
}

// Tick the passed block
bool World::RandomTick(Block* b, Int3& pos) {
    switch(b->type) {
        case BLOCK_GRASS:
        {
            std::uniform_int_distribution<int> dist(-2,2);
            // Random offset
            pos = pos + Int3{dist(rng),dist(rng),dist(rng)};
            Block* nb = GetBlock(pos);
            if (nb && nb->type == BLOCK_DIRT) {
                Block* ab = GetBlock(pos+Int3{0,1,0});
                if (ab && ab->type == BLOCK_AIR) {
                    nb->type = BLOCK_GRASS;
                    return true;
                }
            }
            break;
        }
        case BLOCK_CROP_WHEAT:
        {
            if (b->meta < MAX_CROP_SIZE) {
                b->meta++;
            }
            return true;
        }
    }
    return false;
}