#include "generator.h"

Chunk* Generator::GenerateChunk(int32_t x, int32_t z) {
    Chunk* c = new Chunk();
    for (uint8_t x = 0; x < CHUNK_WIDTH_X; x++) {
        for (uint8_t z = 0; z < CHUNK_WIDTH_X; z++) {
            for (uint8_t y = 0; y < CHUNK_HEIGHT; y++) {
                Block b;
                if (y == 60) {
                    b.type = BLOCK_GRASS;
                    b.meta = 0;
                    b.lightBlock = 0;
                    b.lightSky = 0;
                } else if (y == 59) {
                    b.type = BLOCK_DIRT;
                    b.meta = 0;
                    b.lightBlock = 0;
                    b.lightSky = 0;
                } else if (y > 0 && y < 59) {
                    b.type = BLOCK_STONE;
                    b.meta = 0;
                    b.lightBlock = 0;
                    b.lightSky = 0;
                } else if (y == 0) {
                    b.type = BLOCK_BEDROCK;
                    b.meta = 0;
                    b.lightBlock = 0;
                    b.lightSky = 0;
                } else {
                    b.lightSky = 15;
                }
                c->blocks[GetBlockIndex(XyzToInt3(x,y,z))] = b;
            }
        }   
    }
    return c;
}