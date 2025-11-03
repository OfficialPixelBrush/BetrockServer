#include "chunk.h"

int8_t Chunk::GetHeightValue(uint8_t x, uint8_t z) {
    x = x%15;
    z = z%15;
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
    if (pos.x < 0 || pos.y < 0 || pos.z < 0 ||
        pos.x >= CHUNK_WIDTH_X || pos.y >= CHUNK_HEIGHT || pos.z >= CHUNK_WIDTH_Z)
        return nullptr;
    return &blocks[pos.y + pos.z * CHUNK_HEIGHT + pos.x * CHUNK_HEIGHT * CHUNK_WIDTH_Z];
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
    if (skyLight) return GetSkyLight(pos);
    return GetBlockLight(pos);
}

int8_t Chunk::GetTotalLight(Int3 pos) {
    int8_t totalLight = GetLight(true, pos);
    /*
    if(totalLight > 0) {
        isLit = true;
    }
    */

    // Assume 0
    //totalLight -= var4;
    int8_t blockLight = GetLight(false, pos);
    if(blockLight > totalLight) {
        totalLight = blockLight;
    }

    return totalLight;
}

void Chunk::SetLight(bool skyLight, Int3 pos, int8_t newLight) {
    if (skyLight) {
        SetSkyLight(newLight, pos);
        return;
    }
    SetBlockLight(newLight, pos);
}

int8_t Chunk::GetBlockType(Int3 pos) {
    if (pos.y < 0 || pos.y >= CHUNK_HEIGHT) return BLOCK_AIR;
    Block* b = this->GetBlock(pos);
    if (!b) return BLOCK_AIR;
    return b->type;
}

void Chunk::SetBlockType(int8_t blockType, Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return;
    b->type = blockType;
    RelightBlock(pos.x, pos.y, pos.z);
}

int8_t Chunk::GetBlockMeta(Int3 pos) {
    if (pos.y < 0 || pos.y >= CHUNK_HEIGHT) return 0;
    Block* b = this->GetBlock(pos);
    if (!b) return 0;
    return b->meta;
}

void Chunk::SetBlockMeta(int8_t blockType, Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return;
    b->meta = blockType;
}


int8_t Chunk::GetBlockLight(Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return 0;
    return b->light >> 4;
}

void Chunk::SetBlockLight(int8_t value, Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return;
    b->light &= 0x0F;
    b->light |= ((value & 0xF) << 4);
}

int8_t Chunk::GetSkyLight(Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return 0;
    return b->light & 0x0F;
}

void Chunk::SetSkyLight(int8_t value, Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return;
    b->light &= 0xF0;
    b->light |= (value & 0xF);
}

void Chunk::SetBlockTypeAndMeta(int8_t blockType, int8_t blockMeta, Int3 pos) {
    Block* b = this->GetBlock(pos);
    if (!b) return;
    b->type = blockType;
    b->meta = blockMeta;
    //RelightBlock(pos.x, pos.y, pos.z);
}

void Chunk::AddTileEntity(std::unique_ptr<TileEntity>&& te) {
    tileEntities.push_back(std::move(te));
}

TileEntity* Chunk::GetTileEntity(Int3 pos) {
    for (auto& ts : tileEntities) {
        if (ts->position == pos) {
            return ts.get();
        }
    }
    return nullptr;
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

std::shared_ptr<CompoundTag> Chunk::GetAsNbt() {
    auto root = std::make_shared<CompoundTag>("");
    auto level = std::make_shared<CompoundTag>("Level");
    root->Put(level);
    level->Put(std::make_shared<ByteArrayTag>("Blocks",     GetBlockTypes() ));
    level->Put(std::make_shared<ByteArrayTag>("Data",       GetBlockMetas() ));
    level->Put(std::make_shared<ByteArrayTag>("BlockLight", GetBlockLights()));
    level->Put(std::make_shared<ByteArrayTag>("SkyLight",   GetSkyLights()  ));
    level->Put(std::make_shared<ByteTag>("TerrainPopulated", this->state == ChunkState::Populated));
    level->Put(std::make_shared<IntTag>("xPos",xPos));
    level->Put(std::make_shared<IntTag>("zPos",zPos));
    auto tileEntitiesNbt = std::make_shared<ListTag>("TileEntities");
    level->Put(tileEntitiesNbt);
    for (auto& te : tileEntities) {
        auto subtag = std::make_shared<CompoundTag>("TileEntities");
        // Shared between all tile entities
        subtag->Put(std::make_shared<IntTag>("x", te->position.x));
        subtag->Put(std::make_shared<IntTag>("y", te->position.y));
        subtag->Put(std::make_shared<IntTag>("z", te->position.z));
        subtag->Put(std::make_shared<StringTag>("id", te->type));
        // TODO: Add NBT Writeability to the TEs themselves
        if (te->type == TILEENTITY_SIGN) {
            auto sign = static_cast<SignTile*>(te.get()); 
            subtag->Put(std::make_shared<StringTag>("Text1", sign->lines[0]));
            subtag->Put(std::make_shared<StringTag>("Text2", sign->lines[1]));
            subtag->Put(std::make_shared<StringTag>("Text3", sign->lines[2]));
            subtag->Put(std::make_shared<StringTag>("Text4", sign->lines[3]));
        }
        tileEntitiesNbt->Put(subtag);
    }
    return root;
}

void Chunk::ReadFromNbt(std::shared_ptr<CompoundTag> readRoot) {
    auto root = std::dynamic_pointer_cast<CompoundTag>(readRoot);
    auto level = std::dynamic_pointer_cast<CompoundTag>(root->Get("Level"));
    
    const size_t blockDataSize  = (CHUNK_WIDTH_X * CHUNK_WIDTH_Z *  CHUNK_HEIGHT   );
    const size_t nibbleDataSize = (CHUNK_WIDTH_X * CHUNK_WIDTH_Z * (CHUNK_HEIGHT/2));
    // Block Data
    auto blockData = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("Blocks"))->GetData();
    for (size_t i = 0; i < blockDataSize; i++) {
        blocks[i].type = blockData[i];
    }
    // Block Metadata
    auto metaData = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("Data"))->GetData();
    for (size_t i = 0; i < nibbleDataSize; i++) {
        blocks[i*2  ].meta = (metaData[i]     )&0xF;
        blocks[i*2+1].meta = (metaData[i] >> 4)&0xF;
    }
    // Block Light
    auto blockLightData = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("BlockLight"))->GetData();
    for (size_t i = 0; i < nibbleDataSize; i++) {
        SetBlockLight((blockLightData[i]     )&0xF, BlockIndexToPosition(i*2 ));
        SetBlockLight((blockLightData[i] >> 4)&0xF, BlockIndexToPosition(i*2+1));
    }
    // Sky Light
    auto skyLightData = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("SkyLight"))->GetData();
    for (size_t i = 0; i < nibbleDataSize; i++) {
        SetSkyLight((skyLightData[i]     )&0xF, BlockIndexToPosition(i*2 ));
        SetSkyLight((skyLightData[i] >> 4)&0xF, BlockIndexToPosition(i*2+1));
    }

    // Load Tile Entity Data
    auto tileEntitiesNbt = std::dynamic_pointer_cast<ListTag>(level->Get("TileEntities"));
    if (tileEntitiesNbt && tileEntitiesNbt->GetNumberOfTags() > 0) {
        for (auto teNbt : tileEntitiesNbt->GetTags()) {
            auto teNbtTag = std::dynamic_pointer_cast<CompoundTag>(teNbt);
            // Shared between all tile entities
            auto xTag = std::dynamic_pointer_cast<IntTag>(teNbtTag->Get("x"));
            auto yTag = std::dynamic_pointer_cast<IntTag>(teNbtTag->Get("y"));
            auto zTag = std::dynamic_pointer_cast<IntTag>(teNbtTag->Get("z"));
            auto typeTag = std::dynamic_pointer_cast<StringTag>(teNbtTag->Get("id"));
            if (!xTag || !yTag || !zTag || !typeTag) continue;

            int x = xTag->GetData();
            int y = yTag->GetData();
            int z = zTag->GetData();
            std::string type = typeTag->GetData();
            if (type == TILEENTITY_SIGN) {
                std::array<std::string, 4> lines;
                lines[0] = std::dynamic_pointer_cast<StringTag>(teNbtTag->Get("Text1"))->GetData();
                lines[1] = std::dynamic_pointer_cast<StringTag>(teNbtTag->Get("Text2"))->GetData();
                lines[2] = std::dynamic_pointer_cast<StringTag>(teNbtTag->Get("Text3"))->GetData();
                lines[3] = std::dynamic_pointer_cast<StringTag>(teNbtTag->Get("Text4"))->GetData();
                AddTileEntity(std::make_unique<SignTile>(Int3{x, y, z}, lines));
                continue;
            }
        }
    }

    auto terrainPopulated = std::dynamic_pointer_cast<ByteTag>(level->Get("TerrainPopulated"))->GetData();
    if (terrainPopulated) state = ChunkState::Populated;
}


// Get all the Block Data of a Chunk as an array
std::array<uint8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> Chunk::GetBlockTypes() {
    std::array<uint8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> data;
    int index = 0;
    // BlockData
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < CHUNK_HEIGHT; cY++) {
                data[index++] = GetBlockType(Int3{cX,cY,cZ});
            }
        }
    }
    return data;
}

// Get all the Meta Data of a Chunk as an array
std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> Chunk::GetBlockMetas() {
    std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> data;
    int index = 0;
    // Block Metadata
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                // Default to safe values
                uint8_t b1v = 0;
                uint8_t b2v = 0;
                Block* b1 = GetBlock(cX,cY*2  ,cZ);
                Block* b2 = GetBlock(cX,cY*2+1,cZ);
                if (b1) b1v = b1->meta;
                if (b2) b2v = b2->meta;
                data[index++] = int8_t(b2v << 4 | b1v);
            }
        }
    }
    return data;
}

// Get all the Block Light Data of a Chunk as an array
std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> Chunk::GetBlockLights() {
    std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> data;
    int index = 0;
    // Block Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                // Default to safe values
                uint8_t b1v = GetBlockLight(Int3{cX,cY*2  ,cZ});
                uint8_t b2v = GetBlockLight(Int3{cX,cY*2+1,cZ});
                data[index++] = int8_t(b2v << 4 | b1v);
            }
        }
    }
    return data;
}

// Get all the Sky Light Data of a Chunk as an array
std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> Chunk::GetSkyLights() {
    std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> data;
    int index = 0;

    // Sky Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                // Default to safe values
                uint8_t b1v = GetSkyLight(Int3{cX,cY*2  ,cZ});
                uint8_t b2v = GetSkyLight(Int3{cX,cY*2+1,cZ});
                data[index++] = int8_t(b2v << 4 | b1v);
            }
        }
    }
    return data;
}