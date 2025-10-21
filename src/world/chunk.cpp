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

void Chunk::RelightBlock(int x, int y, int z) {
    int heightMapValue = this->heightMap[z << 4 | x] & 255;
    int heightValue = heightMapValue;
    if(y > heightMapValue) {
        heightValue = y;
    }

    // We decrement heightValue until we hit a fully opaque block
    while(heightValue > 0 && GetTranslucency(this->GetBlockType(Int3{x, heightValue - 1, z})) == 0) {
        //int bType = this->GetBlockType(Int3{x, heightValue - 1, z});
        //std::cout << Int3{x, heightValue - 1, z} << IdToLabel(bType) << ": " << (int)GetTranslucency(bType) << std::endl;
        --heightValue;
    }

    // If heightValue and heightMapValue aren't equal, we recalculate lighting
    if(heightValue != heightMapValue) {
        // This is purely for the Infdev Singleplayer client to update stuff visually
        //this->world->MarkBlocksDirtyVertical(x, z, heightValue, heightMapValue);
        this->heightMap[z << 4 | x] = (int8_t)heightValue;
        int ix;
        int iz;
        if(heightValue < this->lowestBlockHeight) {
            this->lowestBlockHeight = heightValue;
        } else {
            y = 127;

            for(ix = 0; ix < 16; ++ix) {
                for(iz = 0; iz < 16; ++iz) {
                    if((this->heightMap[iz << 4 | ix] & 255) < y) {
                        y = this->heightMap[iz << 4 | ix] & 255;
                    }
                }
            }

            this->lowestBlockHeight = y;
        }

        y = (this->xPos << 4) + x;
        ix = (this->zPos << 4) + z;
        if(heightValue < heightMapValue) {
            for(iz = heightValue; iz < heightMapValue; ++iz) {
                this->SetLight(true,Int3{x, iz, z}, 15);
            }
        } else {
            this->world->AddToLightQueue(true, Int3{y, heightMapValue, ix}, Int3{y, heightValue, ix});

            for(iz = heightMapValue; iz < heightValue; ++iz) {
                this->SetLight(true,Int3{x, iz, z}, 0);
            }
        }

        iz = 15;

        while(heightValue > 0 && iz > 0) {
            --heightValue;
            heightMapValue = GetTranslucency(
                this->GetBlockType(Int3{x, heightValue, z})
            );
            if(heightMapValue == 0) {
                heightMapValue = 1;
            }

            iz -= heightMapValue;
            if(iz < 0) {
                iz = 0;
            }

            this->SetLight(true,Int3{x, heightValue, z}, iz);
            this->world->SpreadLight(true, Int3{y, heightValue, ix}, -1);
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
    if (pos.x < 0 || pos.x >= 16 || pos.z < 0 || pos.z >= 16) {
        // Out of bounds
        return false;
    }
    //if (!heightMap) return false;
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
    if (!b) {
        //std::cout << "NO BLOCK AT " << pos << std::endl;
        return BLOCK_AIR;
    }
    return b->type;
}

void Chunk::SetBlockType(int8_t blockType, Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return;
    b->type = blockType;
    RelightBlock(pos.x, pos.y, pos.z);
}

void Chunk::AddTileEntity(std::unique_ptr<TileEntity>&& te) {
    tileEntities.push_back(std::move(te));
}

std::vector<TileEntity*> Chunk::GetTileEntities() {
    std::vector<TileEntity*> tes;
    for (const auto& te : tileEntities) {
        tes.push_back(te.get());
    }
    return tes;
}

std::vector<SignTile*> Chunk::GetSigns() {
    std::vector<SignTile*> signs;
    for (const auto& te : tileEntities) {
        if (!te || te->type != "Sign") {
            continue;
        }
        if (auto sign = dynamic_cast<SignTile*>(te.get())) {
            signs.push_back(sign);
        }
    }
    return signs;
}