#include "chunk.h"

int8_t Chunk::GetHeightValue(uint8_t x, uint8_t z) {
    return this->heightMap[z << 4 | x] & 255;
}

void Chunk::GenerateHeightMap() {
    int y = 127;
    int x,z;
    for(x = 0; x < 16; ++x) {
        for(z = 0; z < 16; ++z) {
            this->heightMap[z << 4 | x] = -128;
            this->RelightBlock(x, 127, z);
            if((this->heightMap[z << 4 | x] & 255) < y) {
                y = this->heightMap[z << 4 | x] & 255;
            }
        }
    }
    this->lowestBlockHeight = y;

    for(x = 0; x < 16; ++x) {
        for(z = 0; z < 16; ++z) {
            this->UpdateSkylight_do(x, z);
        }
    }

    this->modified = true;
}

void Chunk::RelightBlock(int var1, int var2, int var3) {
    int var4 = this->heightMap[var3 << 4 | var1] & 255;
    int var5 = var4;
    if(var2 > var4) {
        var5 = var2;
    }

    while(var5 > 0 && GetTranslucency(this->GetBlockType(Int3{var1, var5 - 1, var3})) == 0) {
        --var5;
    }

    if(var5 != var4) {
        // This is purely for the Infdev Singleplayer client to update stuff visually
        //this->world->MarkBlocksDirtyVertical(var1, var3, var5, var4);
        this->heightMap[var3 << 4 | var1] = (int8_t)var5;
        int var6;
        int var7;
        if(var5 < this->lowestBlockHeight) {
            this->lowestBlockHeight = var5;
        } else {
            var2 = 127;

            for(var6 = 0; var6 < 16; ++var6) {
                for(var7 = 0; var7 < 16; ++var7) {
                    if((this->heightMap[var7 << 4 | var6] & 255) < var2) {
                        var2 = this->heightMap[var7 << 4 | var6] & 255;
                    }
                }
            }

            this->lowestBlockHeight = var2;
        }

        var2 = (this->xPos << 4) + var1;
        var6 = (this->zPos << 4) + var3;
        if(var5 < var4) {
            for(var7 = var5; var7 < var4; ++var7) {
                this->SetLight(true,Int3{var1, var7, var3}, 15);
            }
        } else {
            this->world->AddToLightQueue(true, Int3{var2, var4, var6}, Int3{var2, var5, var6});

            for(var7 = var4; var7 < var5; ++var7) {
                this->SetLight(true,Int3{var1, var7, var3}, 0);
            }
        }

        var7 = 15;

        while(var5 > 0 && var7 > 0) {
            --var5;
            var4 = GetTranslucency(
                this->GetBlockType(Int3{var1, var5, var3})
            );
            if(var4 == 0) {
                var4 = 1;
            }

            var7 -= var4;
            if(var7 < 0) {
                var7 = 0;
            }

            this->SetLight(true,Int3{var1, var5, var3}, var7);
            this->world->SpreadLight(true, Int3{var2, var5, var6}, -1);
        }

        this->modified = true;
    }
}

void Chunk::UpdateSkylight_do(int x, int z) {
    int height = this->GetHeightValue(x, z);
    x += this->xPos << 4;
    z += this->zPos << 4;
    this->CheckSkylightNeighborHeight(x - 1, z, height);
    this->CheckSkylightNeighborHeight(x + 1, z, height);
    this->CheckSkylightNeighborHeight(x, z - 1, height);
    this->CheckSkylightNeighborHeight(x, z + 1, height);
}

void Chunk::CheckSkylightNeighborHeight(int x, int z, int height) {
    int worldHeight = this->world->GetHeightValue(x, z);
    if(worldHeight > height) {
        this->world->AddToLightQueue(true, Int3{x, height, z}, Int3{x, worldHeight, z});
    } else if(worldHeight < height) {
        this->world->AddToLightQueue(true, Int3{x, worldHeight, z}, Int3{x, height, z});
    }

    this->modified = true;
}

Block* Chunk::GetBlock(Int3 pos) {
    return &blocks[(int32_t)(pos.y + pos.z*CHUNK_HEIGHT + (pos.x*CHUNK_HEIGHT*CHUNK_WIDTH_Z))];
}
Block* Chunk::GetBlock(int32_t x, int8_t y, int32_t z) {
    return GetBlock(Int3{x,y,z});
}

bool Chunk::CanBlockSeeTheSky(Int3 pos) {
    return pos.y >= (this->heightMap[pos.z << 4 | pos.x] & 255);
}

bool Chunk::CanBlockSeeTheSky(int32_t x, int8_t y, int32_t z) {
    return CanBlockSeeTheSky(Int3{x,y,z});
}

int8_t Chunk::GetLight(bool skyLight, Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return 0;
    if (skyLight) return b->lightSky;
    return b->lightBlock;
}

void Chunk::SetLight(bool skyLight, Int3 pos, int8_t newLight) {
    Block* b = this->GetBlock(pos);
    if (!b) return;
    if (skyLight) {
        b->lightSky = newLight;
        return;
    }
    b->lightBlock = newLight;
}

int8_t Chunk::GetBlockType(Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return 0;
    return b->type;
}

void Chunk::SetBlockType(int8_t blockType, Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return;
    b->type = blockType;
}