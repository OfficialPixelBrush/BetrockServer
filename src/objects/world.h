#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <filesystem>

#include "helper.h"
#include "items.h"
#include "generator.h"
#include "worleyPeakGenerator.h"

class World {
    private:
        WorleyPeakGenerator generator;
        std::unordered_map<int64_t, Chunk> chunks;
        int64_t GetChunkHash(int32_t x, int32_t z);
        Int3 DecodeChunkHash(int64_t hash);
        Chunk* GetChunk(int32_t x, int32_t z);
        void AddChunk(int32_t x, int32_t z, Chunk c);
        void RemoveChunk(int32_t x, int32_t z);
    public:
        int64_t seed;
        void Load();
        void Save();
        int GetNumberOfChunks();
        std::unique_ptr<char[]> GetChunkData(Int3 position);
        Chunk GenerateChunk(int32_t x, int32_t z);
        void PlaceBlock(Int3 position, int8_t type, int8_t meta);
        Block BreakBlock(Int3 position);
        Block* GetBlock(Int3 position);
        Int3 FindSpawnableBlock(Int3 position);
        void CalculateColumnLight(int32_t x, int32_t z);
        void CalculateChunkLight(int32_t cX, int32_t cZ);
        void SetSeed(int64_t seed);
        int64_t GetSeed();
};