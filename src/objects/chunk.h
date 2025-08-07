#pragma once
#include <cstdint>
#include "datatypes.h"
#include "blocks.h"

#define CHUNK_HEIGHT 128
#define CHUNK_WIDTH_X 16
#define CHUNK_WIDTH_Z 16

#define CHUNK_DATA_SIZE static_cast<size_t>(CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z * 2.5)

#define OLD_CHUNK_FILE_EXTENSION ".cnk"
#define CHUNK_FILE_EXTENSION ".ncnk"

class Chunk {
    private:
        int8_t heightMap[256];
        uint8_t lowestBlockHeight;
        
    public:
        struct Block blocks[CHUNK_WIDTH_X*CHUNK_WIDTH_Z*CHUNK_HEIGHT];
        // This describes the number of clients that can see this chunk.
        // If this hits 0, the chunk is invisible and can be removed
        // TODO: Actually implement this value!
        uint16_t viewers = 0;

        bool generated = false;

        // A non-populated chunk still needs to be popualated with foliage
        bool populated = false;

        // Set if a chunk was been modified and needs to be re-saved
        bool modified = false;
        int8_t GetHeightValue(uint8_t x, uint8_t z);
        void GenerateHeightMap();
        Block* GetBlock(int8_t x, int8_t y, int8_t z) {
            return &blocks[(int32_t)(y + z*CHUNK_HEIGHT + (x*CHUNK_HEIGHT*CHUNK_WIDTH_Z))];
        }
};