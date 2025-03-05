
#include "lighting.h"

// Calculate a vertical column of light
void CalculateColumnLight(int32_t x, int32_t z, Chunk* c, int32_t& unobstructedLayers) {
    if (!c) {
        return;
    }
    uint8_t skyVisible = 0xF;
    for (int8_t y = CHUNK_HEIGHT-1; y > 0; y--) {
        Int3 position { x,y,z };
        Block* b = &c->blocks[GetBlockIndex(Int3{x,y,z})];
        GetTranslucency(b->type, skyVisible);
        if (!(IsTransparent(b->type) || IsTranslucent(b->type))) {
            // We remember the first layer of blocks that obstructs the sky
            if (unobstructedLayers < y) {
                unobstructedLayers = y;
            }
            b->lightBlock = 0x0;
            skyVisible = 0x0;
        }
        b->lightBlock = GetEmissiveness(b->type);
        b->lightSky = skyVisible;
    }
}

void CalculateSpreadLight(int32_t y, Chunk* c) {
    if (!c) {
        return;
    }

    for (int32_t x = 0; x < CHUNK_WIDTH_X; x++) {
        for (int32_t z = 0; z < CHUNK_WIDTH_Z; z++) {
            Block* b = &c->blocks[GetBlockIndex(Int3{x,y,z})];
            if (!b) {
                continue;
            }
            uint8_t currentLight = b->lightSky; // Assume GetLight() fetches light level
            
            if (currentLight > 1) {
                // Spread light to adjacent blocks
                auto TrySpread = [&](int32_t nx, int32_t nz) {
                    if (nx >= 0 && nx < CHUNK_WIDTH_X && nz >= 0 && nz < CHUNK_WIDTH_Z) {
                        Block* nb = &c->blocks[GetBlockIndex(Int3{nx, y, nz})];
                        if (!nb) {
                            return;
                        }
                        if (IsTransparent(nb->type)) {
                            if (nb->lightSky + 2 <= currentLight) { // Light diminishes
                                nb->lightSky = currentLight-1; // Assume SetLight() updates light
                            }
                        }
                    }
                };

                TrySpread(x - 1, z);
                TrySpread(x + 1, z);
                TrySpread(x, z - 1);
                TrySpread(x, z + 1);
            }
        }
    }
}

// Recalculates all the light in the chunk the block position is found in
void CalculateChunkLight(Chunk* c) {
    if (!c) {
        return;
    }
    // We use this to remember when blocks start appearing,
    // since we don't need to bother dealing with layers of
    // transparent blocks that don't alter the lighting
    int32_t unobstructedLayers = 0;
    // First we do a vertical pass of all lighting
    for (int32_t x = 0; x < CHUNK_WIDTH_X; x++) {
        for (int32_t z = 0; z < CHUNK_WIDTH_Z; z++) {
            CalculateColumnLight(x,z,c,unobstructedLayers);
        }
    }
    /*
    // Then we do a horizontal pass
    for (int32_t y = unobstructedLayers; y >= 0; y--) {
        CalculateSpreadLight(y,c);
    }
    */
}