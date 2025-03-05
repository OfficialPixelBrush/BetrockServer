
#include "lighting.h"

// Calculate a vertical column of light
void CalculateColumnLight(int8_t x, int8_t z, Chunk* c, int8_t& unobstructedLayers) {
    if (!c) {
        return;
    }
    uint8_t skyVisible = 0xF;
    for (int8_t y = CHUNK_HEIGHT-1; y > 0; y--) {
        Block* b = &c->blocks[GetBlockIndex(Int3{x,y,z})];
        if (!b) {
            continue;
        }
        if (!IsTransparent(b->type)) {
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

void CalculateSpreadLight(int8_t y, Chunk* c) {
    if (!c) {
        return;
    }

    for (int8_t x = 0; x < CHUNK_WIDTH_X; x++) {
        for (int8_t z = 0; z < CHUNK_WIDTH_Z; z++) {
            Block* b = &c->blocks[GetBlockIndex(Int3{x,y,z})];
            if (!b) {
                continue;
            }
            uint8_t currentLight = b->lightSky;
            
            if (currentLight > 1) {
                // Spread light to adjacent blocks
                auto TrySpread = [&](int8_t nx, int8_t nz) {
                    if (nx >= 0 && nx < CHUNK_WIDTH_X && nz >= 0 && nz < CHUNK_WIDTH_Z) {
                        Block* nb = &c->blocks[GetBlockIndex(Int3{nx, y, nz})];
                        if (!nb) {
                            return;
                        }
                        //GetTranslucency(b->type, currentLight);
                        if (!IsTransparent(b->type)) {
                            currentLight = 0x0;
                        }
                        if (nb->lightSky + 2 <= currentLight) {
                            // Light diminishes
                            nb->lightSky = currentLight-1;
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

void PropagateLight(World* world, Int3 position, int8_t lightLevel, bool source) {
    if (lightLevel <= 0) return;

    Block* b = world->GetBlock(position);
    if (!IsTransparent(b->type)) return;

    if (b->lightBlock >= lightLevel && !source) return;

    b->lightBlock = lightLevel; // Assign before recursion

    PropagateLight(world, position + Int3{ 1, 0, 0 }, lightLevel - 1, false);
    PropagateLight(world, position + Int3{-1, 0, 0 }, lightLevel - 1, false);
    PropagateLight(world, position + Int3{ 0, 1, 0 }, lightLevel - 1, false);
    PropagateLight(world, position + Int3{ 0,-1, 0 }, lightLevel - 1, false);
    PropagateLight(world, position + Int3{ 0, 0, 1 }, lightLevel - 1, false);
    PropagateLight(world, position + Int3{ 0, 0,-1 }, lightLevel - 1, false);
}


// Recalculates all the light in the chunk the block position is found in
void CalculateChunkLight(Chunk* c) {
    if (!c) {
        return;
    }
    // We use this to remember when blocks start appearing,
    // since we don't need to bother dealing with layers of
    // transparent blocks that don't alter the lighting
    int8_t unobstructedLayers = 0;
    // First we do a vertical pass of all lighting
    for (int8_t x = 0; x < CHUNK_WIDTH_X; x++) {
        for (int8_t z = 0; z < CHUNK_WIDTH_Z; z++) {
            CalculateColumnLight(x,z,c,unobstructedLayers);
        }
    }
    // Then we do a horizontal pass
    for (int32_t y = unobstructedLayers; y >= 0; y--) {
        CalculateSpreadLight(y,c);
    }
}