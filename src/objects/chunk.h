#pragma once
#include <cstdint>
#include "datatypes.h"
#include "blocks.h"
#include "world.h"

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
        void RelightBlock(int var1, int var2, int var3);
        void UpdateSkylight_do(int x, int z);
        void CheckSkylightNeighborHeight(int x, int z, int height);
        int32_t xPos, zPos;
    public:
        int8_t state = ChunkState::Invalid;
        Chunk(World* world, int32_t cX, int32_t cZ) : world(world), xPos(cX), zPos(cZ) {}
        struct Block blocks[CHUNK_WIDTH_X*CHUNK_WIDTH_Z*CHUNK_HEIGHT];
        // This describes the number of clients that can see this chunk.
        // If this hits 0, the chunk is invisible and can be removed
        // TODO: Actually implement this value!
        uint16_t viewers = 0;

        // Set if a chunk was been modified and needs to be re-saved
        bool modified = false;
        int8_t GetHeightValue(uint8_t x, uint8_t z);
        void GenerateHeightMap();
        Block* GetBlock(Int3 pos);
        Block* GetBlock(int32_t x, int8_t y, int32_t z);
        bool CanBlockSeeTheSky(Int3 pos);
        bool CanBlockSeeTheSky(int32_t x, int8_t y, int32_t z);
        void SetLight(bool skyLight, Int3 pos, int8_t newLight);
        int8_t GetLight(bool skyLight, Int3 pos);
        void SetBlockType(int8_t blockType, Int3 pos);
        int8_t GetBlockType(Int3 pos);
};