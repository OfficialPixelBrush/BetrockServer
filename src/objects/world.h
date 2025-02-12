#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <filesystem>

#include "helper.h"
#include "blocks.h"
#include "generator.h"
#include "config.h"
#include "region.h"

class World {
    private:
        std::unordered_map<int64_t, Chunk> chunks;
        Chunk* GetChunk(int32_t x, int32_t z);
        void RemoveChunk(int32_t x, int32_t z);
    public:
        int64_t seed;
        void Load(const std::string &extra = "");
        void Save(const std::string &extra = "");
        int GetNumberOfChunks();
        std::unique_ptr<char[]> GetChunkData(Int3 position);
        //void GenerateChunk(int32_t x, int32_t z);
        void PlaceBlock(Int3 position, int8_t type, int8_t meta);
        Block BreakBlock(Int3 position);
        Block* GetBlock(Int3 position);
        Int3 FindSpawnableBlock(Int3 position);
        void AddChunk(int32_t x, int32_t z, Chunk c);
        // Legacy
        void LoadOld(const std::string &extra);
};