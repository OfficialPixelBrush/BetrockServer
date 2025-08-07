#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <fstream>
#include <memory>
#include <cstdint>
#include <filesystem>
#include <random>

#include "helper.h"
#include "blocks.h"
#include "generator.h"
#include "config.h"

class World {
    private:
        std::unordered_map<long, std::unique_ptr<Chunk>> chunks;
        std::filesystem::path dirPath;
        void RemoveChunk(int32_t x, int32_t z);
        std::random_device dev;
        std::mt19937 rng;
        bool RandomTick(Block* b, Int3& pos);
    public:
        World(const std::string &extra = "");
        int64_t seed;
        void Save();
        int GetNumberOfChunks();
        int8_t GetHeightValue(int32_t x, int32_t z);
        std::unique_ptr<char[]> GetChunkData(Int3 position);
        std::array<int8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> GetChunkBlocks(Chunk* c);
        std::array<int8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> GetChunkMeta(Chunk* c);
        std::array<int8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> GetChunkBlockLight(Chunk* c);
        std::array<int8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> GetChunkSkyLight(Chunk* c);
        void PlaceBlock(Int3 position, int8_t type = 0, int8_t meta = 0, bool sendUpdate = true);
        Block* BreakBlock(Int3 position, bool sendUpdate = true);
        Block* GetBlock(Int3 position);
        Chunk* GetChunk(int32_t x, int32_t z);
        bool IsChunkPopulated(int32_t x, int32_t z);
        bool IsChunkGenerated(int32_t x, int32_t z);
        int8_t GetSkyLight(Int3 position);
        void SetSkyLight(Int3 position, int8_t level);
        void UpdateBlock(Int3 position, Block* b);
        Int3 FindSpawnableBlock(Int3 position);
        Chunk* AddChunk(int32_t x, int32_t z, Chunk c);
        void FreeUnseenChunks();
        void SaveChunk(int32_t x, int32_t z, Chunk* chunk);
        Chunk* LoadChunk(int32_t x, int32_t z);
        Chunk* LoadOldChunk(int32_t x, int32_t z);
        bool ChunkFileExists(int32_t x, int32_t z, std::string extension = std::string(CHUNK_FILE_EXTENSION));
        bool ChunkExists(int32_t x, int32_t z);
        void TickChunks();
        bool InteractWithBlock(Int3 pos);
};