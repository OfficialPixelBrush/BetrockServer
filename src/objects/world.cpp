#include "world.h"

int World::GetNumberOfChunks() {
    return chunks.size();
}

void World::Load() {
    std::filesystem::path dirPath = "world";

    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return;
    }

    uint loadedChunks = 0;

    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        // Check if the entry is a regular file and has a .cnk extension
        if (entry.is_regular_file() && entry.path().extension() == ".cnk") {
            //std::cout << "Loading file: " << entry.path() << std::endl;
            std::vector<std::string> chunkName;


            std::string s;
            std::stringstream ss(entry.path());

            while (getline(ss, s, ',')) {
                // store token string in the vector
                chunkName.push_back(s);
            }
            Int3 pos {
                std::stoi(std::to_string(s[0])),
                0,
                std::stoi(std::to_string(s[1]))
            };
            //std::cout << pos << std::endl;

            // You can load the chunk here based on the file path
            // Example: LoadChunk(entry.path());

            loadedChunks++;
        }
    }
    std::cout << "Loaded " << loadedChunks << " from Disk" << std::endl;
}

void World::Save() {
    std::filesystem::path dirPath = "world";

    if (std::filesystem::create_directory(dirPath)) {
        std::cout << "Directory created: " << dirPath << '\n';
    } else {
        std::cout << "Failed to create directory or it already exists.\n";
    }

    uint savedChunks = 0;
    for (const auto& pair : chunks) {
        const int64_t& hash = pair.first;
        const Chunk& chunk = pair.second;
        Int3 pos = DecodeChunkHash(hash);
        //std::cout << "Chunk at " << pos << std::endl;

        std::filesystem::path filePath = dirPath / (std::to_string(pos.x) + "," + std::to_string(pos.z) + ".cnk");

        std::ofstream chunkFile (filePath);
        if (!chunkFile) {
            std::cerr << "Failed to save chunk at " << pos.x << ", " << pos.z << '\n';
            continue;
        }

        // Acquire existing chunk data
        std::vector<uint8_t> chunkData = GetChunkData(pos);
        // Add it to the list of chunks to be compressed and sent
        if (chunkData.empty()) {
            std::cout << "Failed to get Chunk " << pos << std::endl;
            continue;
        }

        size_t compressedSize = 0;
        char* chunkBinary = CompressChunk(chunkData, compressedSize);
        
        if (!chunkBinary || compressedSize == 0) {		
            std::cout << "Failed to compress Chunk " << pos << std::endl;
            continue;
        }

        chunkFile.write(chunkBinary, compressedSize);
        chunkFile.close();
        savedChunks++;

        delete [] chunkBinary;
    }
    std::cout << "Saved " << savedChunks << " to Disk" << std::endl;
}

void World::SetSeed(int64_t seed) {
    this->seed = seed;
    generator.seed = seed;
}

int64_t World::GetSeed() {
    return this->seed;
}

int64_t World::GetChunkHash(int32_t x, int32_t z) {
    return ((int64_t)x << 32) | (z & 0xFFFFFFFF);
}

Int3 World::DecodeChunkHash(int64_t hash) {
    return Int3 {
        (hash >> 32),
        0,
        hash & 0xFFFFFFFF
    };
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

void World::PlaceBlock(Int3 position, int16_t block) {
    // Get Block Position within Chunk
    Block* b = GetBlock(position);
    b->type = block;
    b->lightBlock = 0x0;
    b->lightSky = 0x0;
    CalculateColumnLight(position.x,position.z);
}

void World::BreakBlock(Int3 position) {
    // Break Block Position within Chunk
    GetBlock(position)->type = 0;
    CalculateColumnLight(position.x,position.z);
}

Block* World::GetBlock(Int3 position) {
    // Get Block Position within Chunk
    int32_t cX = position.x >> 4;
    int32_t cZ = position.z >> 4;
    int8_t bX = position.x & 0xF;
    int8_t bZ = position.z & 0xF;
    return &chunks[GetChunkHash(cX, cZ)].blocks[GetBlockIndex(XyzToInt3(bX,(int8_t)position.y,bZ))];
}

void World::CalculateColumnLight(int32_t x, int32_t z) {
    uint8_t skyVisible = 0xF;
    for (int8_t y = CHUNK_HEIGHT-1; y > 0; y--) {
        Int3 position { x,y,z };
        Block* b = GetBlock(position);
        GetTranslucency(b->type, skyVisible);
        if (!(IsTransparent(b->type) || IsTranslucent(b->type))) {
            b->lightBlock = 0x0;
            skyVisible = 0x0;
        }
        b->lightBlock = GetEmissiveness(b->type);
        b->lightSky = skyVisible;
    }
}

// Recalculates all the light in the chunk the block position is found in
void World::CalculateChunkLight(int32_t cX, int32_t cZ) {
    int32_t startX = cX*CHUNK_WIDTH_X;
    int32_t startZ = cZ*CHUNK_WIDTH_Z;

    for (int32_t x = startX; x < CHUNK_WIDTH_X+startX; x++) {
        for (int32_t z = startZ; z < CHUNK_WIDTH_Z+startZ; z++) {
            CalculateColumnLight(x,z);
        }
    }
}

// TODO: Probably more effective to turn this into a char array,
// especially since it's always the same size!!
std::vector<uint8_t> World::GetChunkData(Int3 position) {
    std::vector<uint8_t> bytes;
    Chunk* c = GetChunk(position.x,position.z);
    if (!c) {
        return bytes;
    }
    // BlockData
    for (int cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int cY = 0; cY < CHUNK_HEIGHT; cY++) {
                Block b = c->blocks[GetBlockIndex(XyzToInt3(cX,cY,cZ))];
                bytes.push_back(b.type);
            }
        }
    }

    // Block Metadata
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2  ,cZ))];
                Block b2 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2+1,cZ))];
                bytes.push_back(b2.meta << 4 | b1.meta);
            }
        }
    }

    // Block Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2  ,cZ))];
                Block b2 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2+1,cZ))];
                bytes.push_back(b2.lightBlock << 4 | b1.lightBlock);
            }
        }
    }

    // Sky Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (int8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block b1 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2  ,cZ))];
                Block b2 = c->blocks[GetBlockIndex(XyzToInt3(cX,cY*2+1,cZ))];
                bytes.push_back(b2.lightSky << 4 | b1.lightSky);
            }
        }
    }
    return bytes;
}

Chunk World::GenerateChunk(int32_t x, int32_t z) {
    Chunk c = generator.GenerateChunk(x,z);
    AddChunk(x,z,c);
    CalculateChunkLight(x,z);
    return c;
}

Int3 World::FindSpawnableBlock(Int3 position) {
    bool aboveBedrock = false;
    bool skyVisible = true;
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        position.y = y;
        int16_t type = GetBlock(position)->type;
        if (type == BLOCK_BEDROCK) {
            aboveBedrock = true;
            break;
        }
    }
    if (!aboveBedrock) {
        std::cout << "Floor of spawn isn't Bedrock, defaulting.";
        return position;
    }

    for (int y = CHUNK_HEIGHT-1; y >= 0; --y) {
        position.y = y;
        int16_t type = GetBlock(position)->type;
        if (type != BLOCK_AIR) {
            // The position above this block is air
            position.y++;
            return position;
        }
    }


        
    std::cout << "Found no suitable place to spawn, defaulting." << std::endl;
    return position;
}