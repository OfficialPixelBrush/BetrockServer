
#include "lighting.h"

void CalculateColumnLight(int32_t x, int32_t z, Chunk* c) {
    if (!c) {
        return;
    }
    uint8_t skyVisible = 0xF;
    for (int8_t y = CHUNK_HEIGHT-1; y > 0; y--) {
        Int3 position { x,y,z };
        Block* b = &c->blocks[GetBlockIndex(Int3{x,y,z})];
        GetTranslucency(b->type, skyVisible);
        if (!(IsTransparent(b->type) || IsTranslucent(b->type))) {
            b->lightBlock = 0x0;
            skyVisible = 0x0;
        }
        b->lightBlock = GetEmissiveness(b->type);
        b->lightSky = skyVisible;
    }
}

// Recalculates all the light in the chunk the block position is found in
void CalculateChunkLight(Chunk* c) {
    if (!c) {
        return;
    }
    for (int32_t x = 0; x < CHUNK_WIDTH_X; x++) {
        for (int32_t z = 0; z < CHUNK_WIDTH_Z; z++) {
            CalculateColumnLight(x,z,c);
        }
    }
}