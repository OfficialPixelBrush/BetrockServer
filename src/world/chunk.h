#pragma once
#include <cstdint>
#include "datatypes.h"
#include "blocks.h"
#include "world.h"
#include "tileEntity.h"
#include "nbt.h"

class World;

enum ChunkState : int8_t {
    Invalid,
    Generated,
    Populated
};

class Chunk {
    private:
        int16_t heightMap[256];
        uint8_t lowestBlockHeight;
        World* world;
        int32_t xPos, zPos;
        std::vector<std::unique_ptr<TileEntity>> tileEntities;
        
        void RelightBlock(int var1, int var2, int var3);
        void UpdateSkylight_do(int x, int z);
        void CheckSkylightNeighborHeight(int x, int z, int height);
    public:
        int8_t state = ChunkState::Invalid;
        struct Block blocks[CHUNK_WIDTH_X*CHUNK_WIDTH_Z*CHUNK_HEIGHT];
        bool modified = false;
        std::shared_ptr<CompoundTag> GetAsNbt();
        void ReadFromNbt(std::shared_ptr<CompoundTag> readRoot);

        Chunk(World* world, int32_t cX, int32_t cZ) : world(world), xPos(cX), zPos(cZ) {}
        int8_t GetHeightValue(uint8_t x, uint8_t z);
        void GenerateHeightMap();
        Block* GetBlock(Int3 pos);
        Block* GetBlock(int32_t x, int8_t y, int32_t z);
        bool CanBlockSeeTheSky(Int3 pos);
        bool CanBlockSeeTheSky(int32_t x, int8_t y, int32_t z);
        void SetLight(bool skyLight, Int3 pos, int8_t newLight);
        int8_t GetLight(bool skyLight, Int3 pos);
        int8_t GetTotalLight(Int3 pos);
        void SetBlockType(int8_t blockType, Int3 pos);
        int8_t GetBlockType(Int3 pos);
        void AddTileEntity(std::unique_ptr<TileEntity>&& te);
        TileEntity* GetTileEntity(Int3 pos);
        std::vector<TileEntity*> GetTileEntities();
        std::vector<SignTile*> GetSigns();
        std::array<uint8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z>     GetBlockTypes();
        std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> GetBlockMetas();
        std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> GetBlockLights();
        std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> GetSkyLights();
};