#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>

#include "helper.h"
#include "items.h"
#include "generator.h"

class World {
    private:
        Generator generator;
        std::unordered_map<int64_t, Chunk> chunks;
        int64_t GetChunkHash(int32_t x, int32_t z);
        Chunk* GetChunk(int32_t x, int32_t z);
        void AddChunk(int32_t x, int32_t z, Chunk c);
        void RemoveChunk(int32_t x, int32_t z);
    public:
        std::vector<uint8_t> GetChunkData(Int3 position);
        Chunk* GenerateChunk(int32_t x, int32_t z);
        void PlaceBlock(Int3 position, int16_t block);
        void BreakBlock(Int3 position);
        int16_t GetBlock(Int3 position);
        Int3 FindSpawnableBlock(Int3 position);
};