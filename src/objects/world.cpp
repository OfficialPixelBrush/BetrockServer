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
        const Chunk& chunk = pair.second; // Keep it as a reference
    
        Int3 pos = DecodeChunkHash(hash);
        SaveChunk(pos.x, pos.z, &chunk); // No need to dereference
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
        for (const auto& player : Betrock::Server::Instance().GetConnectedPlayers()) {  // Assuming players is a vector or container of Player objects
            if (std::find(player->visibleChunks.begin(), player->visibleChunks.end(), DecodeChunkHash(hash)) != player->visibleChunks.end()) {
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
    return true;
}

void World::SaveChunk(int32_t x, int32_t z, const Chunk* chunk) {
    Int3 pos = Int3{x,0,z};
    //std::cout << "Chunk at " << pos << std::endl;

    std::filesystem::path filePath = dirPath / (std::to_string(pos.x) + "," + std::to_string(pos.z) + ".cnk");

    std::ofstream chunkFile (filePath);
    if (!chunkFile) {
        std::cerr << "Failed to save chunk at " << pos.x << ", " << pos.z << '\n';
        return;
    }

    // Acquire existing chunk data
    auto chunkData = GetChunkData(pos);
    // Add it to the list of chunks to be compressed and sent
    if (!chunkData) {
        std::cout << "Failed to get Chunk " << pos << std::endl;
        return;
    }

    size_t compressedSize = 0;
    auto chunkBinary = CompressChunk(chunkData.get(), compressedSize);
    
    if (!chunkBinary || compressedSize == 0) {		
        std::cout << "Failed to compress Chunk " << pos << std::endl;
        return;
    }

    chunkFile.write(chunkBinary.get(), compressedSize);
    chunkFile.close();
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