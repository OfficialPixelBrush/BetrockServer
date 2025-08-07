#include "chunk.h"

int8_t Chunk::GetHeightValue(uint8_t x, uint8_t z) {
    return this->heightMap[z << 4 | x] & 255;
}

void Chunk::GenerateHeightMap() {
    int var2;
    int var3;
    for(var2 = 0; var2 < 16; ++var2) {
        for(var3 = 0; var3 < 16; ++var3) {
            this->heightMap[var3 << 4 | var2] = -128;
            for (int y = CHUNK_HEIGHT-1; y >= 0; --y) {
                Block* b = this->GetBlock(var2,y,var3);
                if (!b) continue;
                if (b->type != BLOCK_AIR) {
                    this->heightMap[var3 << 4 | var2] = y+1;
                    break;
                }
            }
        }
    }

    this->modified = true;
}