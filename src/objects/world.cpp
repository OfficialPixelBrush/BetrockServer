#include "world.h"

#include "server.h"

int World::GetNumberOfChunks() {
    return chunks.size();
}

bool World::ChunkFileExists(int32_t x, int32_t z) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return false;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + ".cnk");

    // Check if the entry file exists and has a .cnk extension
    if (std::filesystem::exists(entryPath) && std::filesystem::is_regular_file(entryPath) && entryPath.extension() == ".cnk") {
        return true;
    }
    return false;
}

bool World::ChunkExists(int32_t x, int32_t z) {
    return chunks.contains(GetChunkHash(x,z));
}

World::World(const std::string& extra) {
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

void World::Save() {
    uint savedChunks = 0;
    for (const auto& pair : chunks) {
        const int64_t& hash = pair.first;
        const Chunk& chunk = pair.second;
    
        Int3 pos = DecodeChunkHash(hash);
        SaveChunk(pos.x, pos.z, &chunk);
        savedChunks++;
    }
    std::cout << "Saved " << savedChunks << " Chunks to Disk" << std::endl;
}

Chunk* World::GetChunk(int32_t x, int32_t z) {
    auto it = chunks.find(GetChunkHash(x, z));
    if (it != chunks.end()) {
        return &it->second; // Return a pointer to the found chunk
    }
    return nullptr; // Return nullptr if no valid object is found
}

void World::AddChunk(int32_t x, int32_t z, Chunk c) {
    chunks[GetChunkHash(x,z)] = c;
}

void World::RemoveChunk(int32_t x, int32_t z) {
    chunks.erase(GetChunkHash(x,z));
}

void World::FreeUnseenChunks() {
    std::vector<Int3> chunksToRemove;

    for (const auto& pair : chunks) {
        const int64_t& hash = pair.first;
        const Chunk& chunk = pair.second;
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

bool World::LoadChunk(int32_t x, int32_t z) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return false;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + ".cnk");

    // Check if the entry file exists and has a .cnk extension
    if (!ChunkFileExists(x,z)) {
        std::cout << "File doesn't exist!" << std::endl;
        return false;
    }

    // TODO: We can know the uncompressed size beforehand, since its always the same.
    auto readRoot = NbtReadFromFile(entryPath,NBT_ZLIB,50);
    //readRoot->NbtPrintData();

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
}

void World::SaveChunk(int32_t x, int32_t z, const Chunk* chunk) {
    if (!chunk) {
        //
    }
    // Update Chunklight before saving
    CalculateChunkLight(GetChunk(x,z));
    Int3 pos = Int3{x,0,z};

    std::filesystem::path filePath = dirPath / (std::to_string(pos.x) + "," + std::to_string(pos.z) + ".cnk");

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
}

void World::PlaceBlock(Int3 position, int8_t type, int8_t meta) {
    // Get Block Position within Chunk
    Block* b = GetBlock(position);
    b->type = type;
    b->meta = meta;
    b->lightBlock = GetEmissiveness(b->type);
    // This needs to be recalculated
    b->lightSky = (IsTranslucent(b->type) || IsTransparent(b->type))*0xF;
    //CalculateColumnLight(position.x,position.z,GetChunk(position.x>>5,position.z>>5));
}

Block World::BreakBlock(Int3 position) {
    // Break Block Position within Chunk
    Block b = *GetBlock(position);
    GetBlock(position)->type = 0;
    //CalculateColumnLight(position.x,position.z);
    return b;
}

Block* World::GetBlock(Int3 position) {
    // Get Block Position within Chunk
    int32_t cX = position.x >> 4;
    int32_t cZ = position.z >> 4;
    int8_t bX = position.x & 0xF;
    int8_t bZ = position.z & 0xF;
    return &chunks[GetChunkHash(cX, cZ)].blocks[GetBlockIndex(XyzToInt3(bX,(int8_t)position.y,bZ))];
}

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
                Block b = c->blocks[GetBlockIndex(XyzToInt3(cX,cY,cZ))];
                bytes[index] = b.type;
                index++;
            }
        }
    }

    // Block Metadata
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2  ,cZ))];
                Block b2 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2+1,cZ))];
                bytes[index] = (b2.meta << 4 | b1.meta);
                index++;
            }
        }
    }

    // Block Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2  ,cZ))];
                Block b2 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2+1,cZ))];
                bytes[index] = (b2.lightBlock << 4 | b1.lightBlock);
                index++;
            }
        }
    }

    // Sky Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2  ,cZ))];
                Block b2 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2+1,cZ))];
                bytes[index] = (b2.lightSky << 4 | b1.lightSky);
                index++;
            }
        }
    }
    return bytes;
}

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
                Block b = c->blocks[GetBlockIndex(XyzToInt3(cX,cY,cZ))];
                data[index] = b.type;
                index++;
            }
        }
    }
    return data;
}

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
                Block b1 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2  ,cZ))];
                Block b2 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2+1,cZ))];
                data[index] = (b2.meta << 4 | b1.meta);
                index++;
            }
        }
    }
    return data;
}

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
                Block b1 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2  ,cZ))];
                Block b2 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2+1,cZ))];
                data[index] = (b2.lightBlock << 4 | b1.lightBlock);
                index++;
            }
        }
    }
    return data;
}

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
                Block b1 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2  ,cZ))];
                Block b2 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2+1,cZ))];
                data[index] = (b2.lightSky << 4 | b1.lightSky);
                index++;
            }
        }
    }
    return data;
}

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