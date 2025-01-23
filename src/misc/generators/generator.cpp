#include "generator.h"

Block Generator::GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible) {
    Block b;
    if (position.y == 0) {
        b.type = BLOCK_BEDROCK;
    } else if (position.y > 0 && position.y < 3) {
        b.type = BLOCK_DIRT;
    } else if (position.y == 3) {
        b.type = BLOCK_GRASS;
    }
    return b;
}

Chunk Generator::GenerateChunk(int32_t cX, int32_t cZ) {
    Chunk c = Chunk();
    for (uint8_t x = 0; x < CHUNK_WIDTH_X; x++) {
        for (uint8_t z = 0; z < CHUNK_WIDTH_X; z++) {
            int8_t blocksSinceSkyVisible = 0;
            for (int8_t y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                Int3 blockPos = Int3 {x,y,z};
                Int3 chunkPos = Int3 {cX,0,cZ};
                Int3 globalPos = LocalToGlobalPosition(chunkPos,blockPos);
                Block b = GenerateBlock(globalPos,blocksSinceSkyVisible);
                if (b.type > BLOCK_AIR) {
                    blocksSinceSkyVisible++;
                }
                
                c.blocks[GetBlockIndex(blockPos)] = b;
            }
        }   
    }
    return c;
}