#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <fstream>
#include <memory>
#include <cstdint>
#include <filesystem>
#include <random>
#include <queue>

#include "helper.h"
#include "blocks.h"
#include "generator.h"
#include "config.h"
#include "chunk.h"
#include "tileEntity.h"
#include "region.h"

class Chunk;
class RegionFile;

struct LightUpdate {
    bool skyLight;
    Int3 posA, posB;
    LightUpdate() :
        skyLight(false), posA(Int3{0,0,0}), posB(Int3{0,0,0}) {}
    LightUpdate(bool skyLight, Int3 posA, Int3 posB) :
        skyLight(skyLight), posA(posA), posB(posB) {}
};
typedef struct LightUpdate LightUpdate;

class World {
    private:
        std::unordered_map<std::string, std::shared_ptr<RegionFile>> openRegions;
        std::mutex regionMutex;

        std::unordered_map<long, std::unique_ptr<Chunk>> chunks;
        std::queue<LightUpdate>lightingToUpdate;
        std::filesystem::path dirPath;
        void RemoveChunk(int32_t x, int32_t z);
        std::random_device dev;
        std::mt19937 rng;
        bool RandomTick(Int3& pos);
        mutable std::shared_mutex lightUpdateMutex;
        mutable std::shared_mutex chunkMutex;
    public:
        int64_t seed;
        void UpdateLightingInfdev();
        World(const std::string &extra = "");

        // Block-related
        void PlaceBlock(Int3 position, int8_t type = BLOCK_AIR, int8_t meta = 0);
        void PlaceBlockUpdate(Int3 position, int8_t type = BLOCK_AIR, int8_t meta = 0, bool sendUpdate = true);
        void PlaceSponge(Int3 position);
        bool BlockExists(Int3 position);

        int8_t GetBlockType(Int3 pos);
        void SetBlockType(int8_t blockType, Int3 pos);
        int8_t GetBlockMeta(Int3 pos);
        void SetBlockMeta(int8_t blockMeta, Int3 pos);

        void SetBlockTypeAndMeta(int8_t blockType, int8_t blockMeta, Int3 pos);

        int8_t GetBlockLight(Int3 pos);
        void SetBlockLight(int8_t value, Int3 pos);
        int8_t GetSkyLight(Int3 pos);
        void SetSkyLight(int8_t value, Int3 pos);


        void UpdateBlock(Int3 position);
        bool InteractWithBlock(Int3 pos);

        // Light-related
        void SpreadLight(bool skyLight, Int3 pos, int limit);
        void AddToLightQueue(bool skyLight, Int3 posA, Int3 posB);
        void SetLight(bool skyLight, Int3 pos,int8_t newLight);
        int8_t GetLight(bool skyLight, Int3 pos);
        int8_t GetTotalLight(Int3 pos);
        bool CanBlockSeeTheSky(Int3 pos);

        // Chunk-related
        std::unique_ptr<char[]> GetChunkData(Int3 position);
        std::vector<SignTile*> GetChunkSigns(Int3 position);
        void TickChunks();
        Chunk* GetChunk(int32_t x, int32_t z);
        bool IsChunkPopulated(int32_t x, int32_t z);
        bool IsChunkGenerated(int32_t x, int32_t z);
        Chunk* AddChunk(int32_t x, int32_t z, std::unique_ptr<Chunk> c);
        void FreeUnseenChunks();
        void SaveChunk(int32_t x, int32_t z, Chunk* chunk);
        Chunk* LoadMcRegionChunk(int32_t cX, int32_t cZ);
        Chunk* LoadOldV2Chunk(int32_t x, int32_t z);
        Chunk* LoadOldChunk(int32_t x, int32_t z);
        bool ChunkFileExists(int32_t x, int32_t z, std::string extension = std::string(CHUNK_FILE_EXTENSION));
        bool ChunkExists(int32_t x, int32_t z);
        
        // Misc
        void Save();
        int GetNumberOfChunks();
        int GetNumberOfPopulatedChunks();
        int8_t GetHeightValue(int32_t x, int32_t z);
        int8_t GetFirstUncoveredBlock(Int3& position);
        void AddTileEntity(std::unique_ptr<TileEntity>&& te);
        TileEntity* GetTileEntity(Int3 pos);
        int GetHighestSolidOrLiquidBlock(int32_t x, int32_t z);
};