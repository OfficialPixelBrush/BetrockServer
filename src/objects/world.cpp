#include "world.h"

int64_t World::GetChunkHash(int32_t x, int32_t z) {
    return ((int64_t)x << 32) | (z & 0xFFFFFFFF);
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
    int8_t bX = position.x & 0xF;
    int8_t bZ = position.z & 0xF;
    int32_t cX = position.x >> 4;
    int32_t cZ = position.z >> 4;
    chunks[GetChunkHash(cX, cZ)].blocks[GetBlockIndex(XyzToInt3(bX,(int8_t)position.y,bZ))].type = block;
}

void World::BreakBlock(Int3 position) {
    // Get Block Position within Chunk
    int8_t bX = position.x & 0xF;
    int8_t bZ = position.z & 0xF;
    int32_t cX = position.x >> 4;
    int32_t cZ = position.z >> 4;
    chunks[GetChunkHash(cX, cZ)].blocks[GetBlockIndex(XyzToInt3(bX,(int8_t)position.y,bZ))].type = 0;
}

int16_t World::GetBlock(Int3 position) {
    // Get Block Position within Chunk
    int8_t bX = position.x & 0xF;
    int8_t bZ = position.z & 0xF;
    int32_t cX = position.x >> 4;
    int32_t cZ = position.z >> 4;
    return (int16_t)chunks[GetChunkHash(cX, cZ)].blocks[GetBlockIndex(XyzToInt3(bX,(int8_t)position.y,bZ))].type;
}

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

Chunk* World::GenerateChunk(int32_t x, int32_t z) {
    Chunk* c = generator.GenerateChunk(x,z);
    if (!c) {
        return nullptr;
    }
    AddChunk(x,z,*c);
    return c;
}

Int3 World::FindSpawnableBlock(Int3 position) {
    bool aboveBedrock = false;
    for (int y = 0; y < CHUNK_HEIGHT; y++) {
        position.y = y;
        int16_t type = GetBlock(position);
        if (type == BLOCK_BEDROCK) {
            aboveBedrock = true;
            continue;
        }
        if (type == BLOCK_AIR && aboveBedrock) {
            return position;
        }
    }
    std::cout << "Found no suitable place to spawn, defaulting." << std::endl;
    return position;
}