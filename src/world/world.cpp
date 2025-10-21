#include "world.h"

#include "server.h"

// Returns the number of Chunks that're currently loaded into memory
int World::GetNumberOfChunks() {
    return chunks.size();
}

int8_t World::GetHeightValue(int32_t x, int32_t z) {
    Chunk* c = GetChunk(x >> 4, z >> 4);
    if (!c) return 0;
    return c->GetHeightValue(x & 15, z & 15);
}

// Checks if a file with a matching position and extension exists
bool World::ChunkFileExists(int32_t x, int32_t z, std::string extension) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return false;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + extension);

    // Check if the entry file exists and has a .cnk extension
    if (std::filesystem::exists(entryPath) && std::filesystem::is_regular_file(entryPath) && entryPath.extension() == extension) {
        return true;
    }
    return false;
}

bool World::ChunkExists(int32_t x, int32_t z) {
    std::unique_lock lock(chunkMutex);
    return chunks.contains(GetChunkHash(x,z));
}

bool World::BlockExists(Int3 pos) {
    return pos.y >= 0 && pos.y < 128 ? ChunkExists(pos.x >> 4, pos.z >> 4) : false;
}

bool World::IsChunkGenerated(int32_t x, int32_t z) {
    Chunk* c = this->GetChunk(x,z);
    if (!c) return false;
    return c->state == ChunkState::Generated;
}

// Checks if the Chunk is populated
bool World::IsChunkPopulated(int32_t x, int32_t z) {
    Chunk* c = this->GetChunk(x,z);
    if (!c) return false;
    return c->state == ChunkState::Populated;
}

// Sets the directory path of the world upon creation
World::World(const std::string& extra)
    : dev(), rng(dev())
{
    std::cout << "CWD: " << std::filesystem::current_path() << "\n";

    dirPath = std::filesystem::current_path() / std::string(Betrock::GlobalConfig::Instance().Get("level-name"));

    // Create dirPath first
    if (!std::filesystem::create_directories(dirPath)) {
        std::cout << "Failed to create: " << dirPath << std::endl;
    }

    // Then dimension sub-directories
    if (!extra.empty()) {
        dirPath /= extra;
        if (!std::filesystem::create_directories(dirPath)) {
            std::cout << "Failed to create: " << dirPath << std::endl;
        }
    }

    // The region folder in that
    dirPath /= "region";
    if (!std::filesystem::create_directories(dirPath)) {
        std::cout << "Failed to create: " << dirPath << std::endl;
    }
}

// Saves all the Chunks that're currently loaded into Memory
void World::Save() {
    for (auto& pair : chunks) {
        int64_t hash = pair.first;
        Chunk* chunk = pair.second.get();

        Int3 pos = DecodeChunkHash(hash);
        SaveChunk(pos.x, pos.z, chunk);
    }
}

// Gets the Chunk Pointer from Memory
Chunk* World::GetChunk(int32_t x, int32_t z) {
    std::unique_lock lock(chunkMutex);
    auto it = chunks.find(GetChunkHash(x, z));
    if (it != chunks.end() && it->second != nullptr)
        return it->second.get();
    return nullptr;
}

// Adds a new Chunk to the world
Chunk* World::AddChunk(int32_t x, int32_t z, std::unique_ptr<Chunk> c) {
    std::unique_lock lock(chunkMutex);
    auto hash = GetChunkHash(x, z);
    chunks[hash] = std::move(c);
    return chunks[hash].get();
}

// Removes a Chunk from the world
void World::RemoveChunk(int32_t x, int32_t z) {
    std::unique_lock lock(chunkMutex);
    chunks.erase(GetChunkHash(x,z));
}

// Removes any chunks that're not visible to any player
void World::FreeUnseenChunks() {
    std::vector<Int3> chunksToRemove;

    for (auto& pair : chunks) {
        const int64_t& hash = pair.first;
        Chunk* chunk = pair.second.get();
        Int3 pos = DecodeChunkHash(hash);
    
        // Check if any player has this chunk hash in their visibleChunks
        bool isVisible = false;
        for (const auto& c : Betrock::Server::Instance().GetConnectedClients()) {
            if (c->ChunkIsVisible(pos)) {
                isVisible = true;
                break;
            }
        }
    
        if (!isVisible) {
            SaveChunk(pos.x, pos.z, chunk);
            chunksToRemove.push_back(pos);
        }
    }    

    // Remove chunks that are not visible to any player
    for (Int3 pos : chunksToRemove) {
        RemoveChunk(pos.x, pos.z);
    }
}

// Load a Chunk into Memory from an NBT-Format file
Chunk* World::LoadChunk(int32_t x, int32_t z) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return nullptr;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + CHUNK_FILE_EXTENSION);

    // Check if the entry file exists and has a .cnk extension
    if (!ChunkFileExists(x,z)) {
        std::cout << "File doesn't exist!" << std::endl;
        return nullptr;
    }

    try {
        // TODO: This estimate is probably overkill
        auto readRoot = NbtReadFromFile(entryPath,NBT_ZLIB,-1,CHUNK_DATA_SIZE*2);
        if (!readRoot) {
            throw std::runtime_error("Unable to read NBT data!");
        }

        auto root = std::dynamic_pointer_cast<CompoundTag>(readRoot);
        auto level = std::dynamic_pointer_cast<CompoundTag>(root->Get("Level"));
        auto blockTag = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("Blocks"));
        auto blocks = blockTag->GetData();
        auto meta = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("Data"))->GetData();
        auto blockLight = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("BlockLight"))->GetData();
        auto skyLight = std::dynamic_pointer_cast<ByteArrayTag>(level->Get("SkyLight"))->GetData();

        auto terrainPopulated = std::dynamic_pointer_cast<ByteTag>(level->Get("TerrainPopulated"))->GetData();

        std::unique_ptr<Chunk> c = std::make_unique<Chunk>(this,x,z);
        const size_t blockDataSize  = (CHUNK_WIDTH_X * CHUNK_WIDTH_Z *  CHUNK_HEIGHT   );
        const size_t nibbleDataSize = (CHUNK_WIDTH_X * CHUNK_WIDTH_Z * (CHUNK_HEIGHT/2));
        // Block Data
        for (size_t i = 0; i < blockDataSize; i++) {
            c->blocks[i].type = blocks[i];
        }
        // Block Metadata
        for (size_t i = 0; i < nibbleDataSize; i++) {
            c->blocks[i*2  ].meta = (meta[i]     )&0xF;
            c->blocks[i*2+1].meta = (meta[i] >> 4)&0xF;
        }
        // Block Light
        for (size_t i = 0; i < nibbleDataSize; i++) {
            c->blocks[i*2  ].lightBlock = (blockLight[i]     )&0xF;
            c->blocks[i*2+1].lightBlock = (blockLight[i] >> 4)&0xF;
        }
        // Sky Light
        for (size_t i = 0; i < nibbleDataSize; i++) {
            c->blocks[i*2  ].lightSky = (skyLight[i]     )&0xF;
            c->blocks[i*2+1].lightSky = (skyLight[i] >> 4)&0xF;
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
                    c->AddTileEntity(std::make_unique<SignTile>(Int3{x, y, z}, lines));
                    continue;
                }
            }
        }

        if (terrainPopulated) c->state = ChunkState::Populated;
        return AddChunk(x,z,std::move(c));
    } catch (const std::exception& e) {
        Betrock::Logger::Instance().Error(e.what());
        return nullptr;
    }
}

// Save a Chunk as an NBT-format file
void World::SaveChunk(int32_t x, int32_t z, Chunk* chunk) {
    if (!chunk || !chunk->modified) {
        //
    }
    // Update Chunklight before saving
    //CalculateChunkLight(GetChunk(x,z));
    Int3 pos = Int3{x,0,z};

    std::filesystem::path filePath = dirPath / (std::to_string(pos.x) + "," + std::to_string(pos.z) + CHUNK_FILE_EXTENSION);

    // Acquire existing chunk data
    auto blocks = GetChunkBlocks(chunk);
    auto meta = GetChunkMeta(chunk);
    auto blockLight = GetChunkBlockLight(chunk);
    auto skyLight = GetChunkSkyLight(chunk);
    auto tileEntities = chunk->GetTileEntities();

    auto root = std::make_shared<CompoundTag>("");
    auto level = std::make_shared<CompoundTag>("Level");
    root->Put(level);
    level->Put(std::make_shared<ByteArrayTag>("Blocks", blocks));
    level->Put(std::make_shared<ByteArrayTag>("Data", meta));
    level->Put(std::make_shared<ByteArrayTag>("BlockLight", blockLight));
    level->Put(std::make_shared<ByteArrayTag>("SkyLight", skyLight));
    level->Put(std::make_shared<ByteTag>("TerrainPopulated", chunk->state == ChunkState::Populated));
    level->Put(std::make_shared<IntTag>("xPos",x));
    level->Put(std::make_shared<IntTag>("zPos",z));
    auto tileEntitiesNbt = std::make_shared<ListTag>("TileEntities");
    level->Put(tileEntitiesNbt);
    for (auto te : tileEntities) {
        auto subtag = std::make_shared<CompoundTag>("TileEntities");
        // Shared between all tile entities
        subtag->Put(std::make_shared<IntTag>("x", te->position.x));
        subtag->Put(std::make_shared<IntTag>("y", te->position.y));
        subtag->Put(std::make_shared<IntTag>("z", te->position.z));
        subtag->Put(std::make_shared<StringTag>("id", te->type));
        if (te->type == TILEENTITY_SIGN) {
            auto sign = static_cast<SignTile*>(te); 
            subtag->Put(std::make_shared<StringTag>("Text1", sign->lines[0]));
            subtag->Put(std::make_shared<StringTag>("Text2", sign->lines[1]));
            subtag->Put(std::make_shared<StringTag>("Text3", sign->lines[2]));
            subtag->Put(std::make_shared<StringTag>("Text4", sign->lines[3]));
        }
        tileEntitiesNbt->Put(subtag);
    }
    
    NbtWriteToFile(filePath,root,NBT_ZLIB);
    chunk->modified = false;
}

void World::PlaceSponge(Int3 position) {
    PlaceBlock(position, BLOCK_SPONGE);
    for (int x = -2; x <= 2; x++) {
        for (int z = -2; z <= 2; z++) {
            for (int y = -2; y <= 2; y++) {
                Block* b = GetBlock(position + Int3{x,y,z});
                if (!b) continue;
                if (
                    b->type == BLOCK_WATER_STILL ||
                    b->type == BLOCK_WATER_FLOWING
                ) {
                    PlaceBlock(position + Int3{x,y,z}, BLOCK_AIR);
                }
            }
        }
    }
}

// Place a block at the passed position
// This position must be within a currently loaded Chunk
void World::PlaceBlock(Int3 position, int8_t type, int8_t meta, bool sendUpdate) {
    LimitBlockCoordinates(position);
    // Get Block Position within Chunk
    Block* b = GetBlock(position);
    if (!b) {
        return;
    }
    b->type = type;
    b->meta = meta;
    b->lightBlock = GetEmissiveness(b->type);
    // This needs to be recalculated
    b->lightSky = (IsTranslucent(b->type) || IsTransparent(b->type))*0xF;
    /*
    if (IsEmissive(b->type)) {
        PropagateLight(this,position,b->lightBlock,true);
    }
    */
    if (sendUpdate) UpdateBlock(position,b);
}

// This is just called "a" in the Source Code
void World::SpreadLight(bool skyLight, Int3 pos, int limit) {
    if(this->BlockExists(pos) && this->GetLight(skyLight, pos) != limit) {
        this->AddToLightQueue(skyLight, pos, pos);
    }
}

// This is just called "a" in the Source Code
void World::AddToLightQueue(bool skyLight, Int3 posA, Int3 posB) {
    std::lock_guard<std::mutex> lock(stackMutex);  // Ensure thread safety
    this->lightingToUpdate.emplace(LightUpdate(skyLight,posA,posB));
}

void World::UpdateLighting() {
    while(true) {
        LightUpdate currentUpdate;
        {
            std::lock_guard<std::mutex> lock(stackMutex);
            if (lightingToUpdate.empty()) break;
            currentUpdate = lightingToUpdate.top();
            lightingToUpdate.pop();
        }

        for(int blockX = currentUpdate.posA.x; blockX <= currentUpdate.posB.x; ++blockX) {
            for(int blockZ = currentUpdate.posA.z; blockZ <= currentUpdate.posB.z; ++blockZ) {
                if(this->BlockExists(Int3{blockX, 0, blockZ})) {
                    for(int blockY = currentUpdate.posA.y; blockY <= currentUpdate.posB.y; ++blockY) {
                        if(blockY >= 0 && blockY < CHUNK_HEIGHT) {
                            int skyLightLevel = this->GetLight(currentUpdate.skyLight, Int3{blockX, blockY, blockZ});
                            int blockType = this->GetBlockType(Int3{blockX, blockY, blockZ});
                            int translucencyLevel = GetTranslucency(blockType);
                            if(translucencyLevel == 0) translucencyLevel = 1;

                            int lightLevel = 0;
                            int inChunkBlockX;
                            int inChunkBlockZ;
                            if(!currentUpdate.skyLight) {
                                lightLevel = GetEmissiveness(blockType);
                            } else {
                                bool updateSkylight;
                                // Check if block is within world bounds
                                if(blockX >= -32000000 && blockZ >= -32000000 && blockX < 32000000 && blockZ <= 32000000) {
                                    // Block is in void
                                    if(blockY < 0) {
                                        updateSkylight = false;
                                    // Block is above build limit
                                    } else if(blockY >= CHUNK_HEIGHT) {
                                        updateSkylight = true;
                                    // Chunk doesn't exist
                                    } else if(!this->ChunkExists(blockX >> 4, blockZ >> 4)) {
                                        updateSkylight = false;
                                    } else {
                                        Chunk* currentChunk = this->GetChunk(blockX >> 4, blockZ >> 4);
                                        inChunkBlockX = blockX & 15;
                                        inChunkBlockZ = blockZ & 15;
                                        updateSkylight = currentChunk->CanBlockSeeTheSky(inChunkBlockX, blockY, inChunkBlockZ);
                                    }
                                } else {
                                    updateSkylight = false;
                                }

                                if(updateSkylight) lightLevel = 15;
                            }

                            int var12;
                            int var18;
                            // Opaque Block
                            if(translucencyLevel >= 15 && lightLevel == 0) {
                                // Probably not actually the block type, but just reused by java
                                blockType = 0;
                            } else {
                                blockType = this->GetLight(currentUpdate.skyLight, Int3{blockX - 1, blockY, blockZ});
                                int var10 = this->GetLight(currentUpdate.skyLight, Int3{blockX + 1, blockY, blockZ});
                                inChunkBlockX = this->GetLight(currentUpdate.skyLight, Int3{blockX, blockY - 1, blockZ});
                                var12 = this->GetLight(currentUpdate.skyLight, Int3{blockX, blockY + 1, blockZ});
                                inChunkBlockZ = this->GetLight(currentUpdate.skyLight, Int3{blockX, blockY, blockZ - 1});
                                var18 = this->GetLight(currentUpdate.skyLight, Int3{blockX, blockY, blockZ + 1});
                                blockType = blockType;
                                if(var10 > blockType) blockType = var10;
                                if(inChunkBlockX > blockType) blockType = inChunkBlockX;
                                if(var12 > blockType) blockType = var12;
                                if(inChunkBlockZ > blockType) blockType = inChunkBlockZ;
                                if(var18 > blockType) blockType = var18;

                                blockType -= translucencyLevel;
                                if(blockType < 0) blockType = 0;
                                if(lightLevel > blockType) blockType = lightLevel;
                            }

                            if(skyLightLevel != blockType) {
                                var18 = blockZ;
                                inChunkBlockZ = blockY;
                                var12 = blockX;
                                bool var17 = currentUpdate.skyLight;
                                //World var16 = var2;
                                // Check if in world bounds
                                if(blockX >= -32000000 && blockZ >= -32000000 && blockX < 32000000 && blockZ <= 32000000 && blockY >= 0 && blockY < 128 && this->ChunkExists(blockX >> 4, blockZ >> 4)) {
                                    Chunk* c = this->GetChunk(blockX >> 4, blockZ >> 4);
                                    if (!c) continue;
                                    c->SetLight(var17, Int3{blockX & 15, blockY, blockZ & 15}, blockType);

                                    //for(skyLightLevel = 0; skyLightLevel < var16.worldAccesses.size(); ++skyLightLevel) {
                                    //    ((IWorldAccess)var16.worldAccesses.get(skyLightLevel)).markBlockAndNeighborsNeedsUpdate(var12, inChunkBlockZ, var18);
                                    //}
                                }

                                --blockType;
                                if(blockType < 0) blockType = 0;

                                this->SpreadLight(currentUpdate.skyLight, Int3{blockX - 1, blockY, blockZ}, blockType);
                                this->SpreadLight(currentUpdate.skyLight, Int3{blockX + 1, blockY, blockZ}, blockType);
                                this->SpreadLight(currentUpdate.skyLight, Int3{blockX, blockY - 1, blockZ}, blockType);
                                this->SpreadLight(currentUpdate.skyLight, Int3{blockX, blockY + 1, blockZ}, blockType);
                                this->SpreadLight(currentUpdate.skyLight, Int3{blockX, blockY, blockZ - 1}, blockType);
                                this->SpreadLight(currentUpdate.skyLight, Int3{blockX, blockY, blockZ + 1}, blockType);
                            }
                        }
                    }
                }
            }
        }
    }
}


// Remove the block and turn it into air
Block* World::BreakBlock(Int3 position, bool sendUpdate) {
    LimitBlockCoordinates(position);
    // Break Block Position within Chunk
    Block* b = GetBlock(position);
    if (!b) {
        return nullptr;
    }
    b->type = 0;
    b->meta = 0;
    if (sendUpdate) UpdateBlock(position,b);
    return b;
}

void World::UpdateBlock(Int3 position, Block* b) {
    if (!b) {
        return;
    }
    std::vector<uint8_t> response;
    Respond::BlockChange(response,position,b->type,b->meta);
    BroadcastToClients(response);
}

// Get the Block at the passed position
Block* World::GetBlock(Int3 position) {
    LimitBlockCoordinates(position);
    // Get Block Position within Chunk
    int32_t cX = position.x >> 4;
    int32_t cZ = position.z >> 4;
    int8_t bX = position.x & 0xF;
    int8_t bZ = position.z & 0xF;
    Chunk* c = GetChunk(cX,cZ);
    if (!c) {
        return nullptr;
    }
    c->modified = true;
    return c->GetBlock(bX,position.y,bZ);
}

// Get the Skylight of a Block at the passed position
int8_t World::GetSkyLight(Int3 position) {
    // Get Block Position within Chunk
    Block* b = GetBlock(position);
    if (b) {
        return b->lightSky;
    }
    return 0;
}

// Set the Skylight of a Block at the passed position
void World::SetSkyLight(Int3 position, int8_t level) {
    // Get Block Position within Chunk
    Block* b = GetBlock(position);
    if (b) {
        b->lightSky = level;
    }
}

std::vector<SignTile*> World::GetChunkSigns(Int3 position) {
    Chunk* c = GetChunk(position.x,position.z);
    if (!c) {
        return {};
    }
    return c->GetSigns();
}

// Get all the block,meta,block light and sky light data of a Chunk in a Binary Format
std::unique_ptr<char[]> World::GetChunkData(Int3 position) {
    auto bytes = std::make_unique<char[]>(CHUNK_DATA_SIZE);
    int index = 0;
    Chunk* c = GetChunk(position.x,position.z);
    if (!c) {
        return nullptr;
    }
    // TODO: Make use of the individual functions!!!
    // BlockData
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < CHUNK_HEIGHT; cY++) {
                Block* b = c->GetBlock(cX,cY,cZ);
                if (!b) continue;
                bytes[index] = b->type;
                index++;
            }
        }
    }

    // Block Metadata
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block* b1 = c->GetBlock(cX,cY*2,cZ);
                Block* b2 = c->GetBlock(cX,cY*2+1,cZ);
                if (!b1 || !b2) continue;
                bytes[index] = (b2->meta << 4 | b1->meta);
                index++;
            }
        }
    }

    // Block Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block* b1 = c->GetBlock(cX,cY*2,cZ);
                Block* b2 = c->GetBlock(cX,cY*2+1,cZ);
                if (!b1 || !b2) continue;
                bytes[index] = (b2->lightBlock << 4 | b1->lightBlock);
                index++;
            }
        }
    }

    // Sky Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                Block* b1 = c->GetBlock(cX,cY*2,cZ);
                Block* b2 = c->GetBlock(cX,cY*2+1,cZ);
                if (!b1 || !b2) continue;
                bytes[index] = (b2->lightSky << 4 | b1->lightSky);
                index++;
            }
        }
    }
    return bytes;
}

// Get all the Block Data of a Chunk as an array
std::array<uint8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> World::GetChunkBlocks(Chunk* c) {
    std::array<uint8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> data;
    if (!c) {
        return data;
    }
    int index = 0;
    // BlockData
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < CHUNK_HEIGHT; cY++) {
                Block* b = c->GetBlock(cX,cY,cZ);
                if (!b) continue;
                data[index] = b->type;
                index++;
            }
        }
    }
    return data;
}

// Get all the Meta Data of a Chunk as an array
std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> World::GetChunkMeta(Chunk* c) {
    std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> data;
    if (!c) {
        return data;
    }
    int index = 0;
    // Block Metadata
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                // Default to safe values
                uint8_t b1v = 0;
                uint8_t b2v = 0;
                Block* b1 = c->GetBlock(cX,cY*2  ,cZ);
                Block* b2 = c->GetBlock(cX,cY*2+1,cZ);
                if (b1) b1v = b1->meta;
                if (b2) b2v = b2->meta;
                data[index++] = int8_t(b2v << 4 | b1v);
            }
        }
    }
    return data;
}

// Get all the Block Light Data of a Chunk as an array
std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> World::GetChunkBlockLight(Chunk* c) {
    std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> data;
    if (!c) {
        return data;
    }
    int index = 0;
    // Block Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                // Default to safe values
                uint8_t b1v = 0;
                uint8_t b2v = 0;
                Block* b1 = c->GetBlock(cX,cY*2  ,cZ);
                Block* b2 = c->GetBlock(cX,cY*2+1,cZ);
                if (b1) b1v = b1->lightBlock;
                if (b2) b2v = b2->lightBlock;
                data[index++] = int8_t(b2v << 4 | b1v);
            }
        }
    }
    return data;
}

// Get all the Sky Light Data of a Chunk as an array
std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> World::GetChunkSkyLight(Chunk* c) {
    std::array<uint8_t, CHUNK_WIDTH_X * (CHUNK_HEIGHT/2) * CHUNK_WIDTH_Z> data;
    if (!c) {
        return data;
    }
    int index = 0;

    // Sky Light
    for (int8_t cX = 0; cX < CHUNK_WIDTH_X; cX++) {
        for (int8_t cZ = 0; cZ < CHUNK_WIDTH_Z; cZ++) {
            for (uint8_t cY = 0; cY < (CHUNK_HEIGHT/2); cY++) {
                // Default to safe values
                uint8_t b1v = 0;
                uint8_t b2v = 0;
                Block* b1 = c->GetBlock(cX,cY*2  ,cZ);
                Block* b2 = c->GetBlock(cX,cY*2+1,cZ);
                if (b1) b1v = b1->lightSky;
                if (b2) b2v = b2->lightSky;
                data[index++] = int8_t(b2v << 4 | b1v);
            }
        }
    }
    return data;
}

// Find the highest possible non-solid block that can see the sky
Int3 World::FindSpawnableBlock(Int3 position) {
    Int3 spawn = position;

    for (int y = CHUNK_HEIGHT-1; y >= 0; --y) {
        spawn.y = y;
        Block* b = GetBlock(spawn);
        if (!b) {
            // There is no chunk to check for blocks
            return position;
        }
        int8_t type = b->type;
        if (type != BLOCK_AIR) {
            // The position above this block is air
            return spawn;
        }
    }

    std::cout << "Found no suitable place to spawn, defaulting." << std::endl;
    return position;
}

// Load an old-format Chunk into Memory from a Binary File
Chunk* World::LoadOldChunk(int32_t x, int32_t z) {
    if (!std::filesystem::exists(dirPath) || !std::filesystem::is_directory(dirPath)) {
        std::cerr << "Directory " << dirPath << " does not exist or is not a directory!" << std::endl;
        return nullptr;
    }

    // Create the chunk entry file path based on x and z coordinates
    std::filesystem::path entryPath = dirPath / (std::to_string(x) + "," + std::to_string(z) + OLD_CHUNK_FILE_EXTENSION);

    // Check if the entry file exists and has a .cnk extension
    if (!ChunkFileExists(x,z,OLD_CHUNK_FILE_EXTENSION)) {
        return nullptr;
    }

    std::ifstream chunkFile (entryPath);
    if (!chunkFile.is_open()) {
        Betrock::Logger::Instance().Warning("Failed to load chunk " + std::string(entryPath));
        return nullptr;
    }      

    // Get the length of the file
    chunkFile.seekg(0, std::ios::end);
    std::streamsize size = chunkFile.tellg();
    chunkFile.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    chunkFile.read(buffer.data(), size);
    char* compressedChunk = buffer.data();

    size_t compressedSize = size;
    size_t decompressedSize = 0;

    auto chunkData = DecompressChunk(compressedChunk,compressedSize,decompressedSize);

    if (!chunkData) {
        Betrock::Logger::Instance().Warning("Failed to decompress " + std::string(entryPath));
        return nullptr;
    }

    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(this,x,z);
    size_t blockDataSize = CHUNK_WIDTH_X*CHUNK_WIDTH_Z*CHUNK_HEIGHT;
    size_t nibbleDataSize = CHUNK_WIDTH_X*CHUNK_WIDTH_Z*(CHUNK_HEIGHT/2);
    for (size_t i = 0; i < decompressedSize; i++) {
        if (i < blockDataSize) {
            // Block Data
            c->blocks[i].type = chunkData[i];
        } else if (
            // Metadata
            i >= blockDataSize &&
            i <  blockDataSize+nibbleDataSize)
        {
            c->blocks[(i%nibbleDataSize)*2  ].meta = (chunkData[i]     )&0xF;
            c->blocks[(i%nibbleDataSize)*2+1].meta = (chunkData[i] >> 4)&0xF;
        } else if (
            // Block Light
            i >= blockDataSize+nibbleDataSize &&
            i <  blockDataSize+(nibbleDataSize*2))
        {
            c->blocks[(i%nibbleDataSize)*2  ].lightBlock = (chunkData[i]     )&0xF;
            c->blocks[(i%nibbleDataSize)*2+1].lightBlock = (chunkData[i] >> 4)&0xF;
        } else if (
            // Sky Light
            i >= blockDataSize+(nibbleDataSize*2) &&
            i <  blockDataSize+(nibbleDataSize*3))
        {
            c->blocks[(i%nibbleDataSize)*2  ].lightSky = (chunkData[i]     )&0xF;
            c->blocks[(i%nibbleDataSize)*2+1].lightSky = (chunkData[i] >> 4)&0xF;
        }
    }
    chunkFile.close();
    Betrock::Logger::Instance().Info("Updated " + std::string(entryPath));
    // Delete the old chunk file
    remove(entryPath);
    return AddChunk(x,z,std::move(c));
}

bool World::InteractWithBlock(Int3 pos) {
    Block* b = GetBlock(pos);
    if (!b) {
        return false;
    }
    if (b->type == BLOCK_TRAPDOOR) {
        b->meta ^= 0b100;
    }
    if (b->type == BLOCK_DOOR_WOOD) {
        b->meta ^= 0b100;
        Int3 nPos = pos;
        if (b->meta & 0b1000) {
            // Interacted with Top
            // Update Bottom
            nPos = pos + Int3{0,-1,0};
        } else {
            // Interacted with Bottom
            // Update Top
            nPos = pos + Int3{0,1,0};
        }
        Block* bb = GetBlock(nPos);
        if (bb && bb->type==b->type) {
            bb->meta ^= 0b100;
            UpdateBlock(nPos,bb);
        }
    }
    UpdateBlock(pos,b);
    return true;
}

// Tick all currently loaded chunks
void World::TickChunks() {
    for (auto& pair : chunks) {
        int64_t hash = pair.first;
        Chunk* chunk = pair.second.get();
        std::uniform_int_distribution<int32_t> dist6(0,CHUNK_WIDTH_X*CHUNK_HEIGHT*CHUNK_WIDTH_Z);
        // Choose a batch of random blocks within a chunk to run RandomTick on
        for (int i = 0; i < 16; i++) {
            int blockIndex = dist6(rng);
            Block* b = &chunk->blocks[blockIndex];
            if (!b) continue;

            Int3 chunkPos = DecodeChunkHash(hash);
            Int3 blockPos = GetBlockPosition(blockIndex);
            Int3 pos = {
                chunkPos.x<<4 | blockPos.x,
                blockPos.y,
                chunkPos.z<<4 | blockPos.z
            };
            // If the block was changed, send this to the clients
            if (!RandomTick(b,pos)) continue;
            
            Block* nb = GetBlock(pos);
            if (nb) UpdateBlock(pos,nb);
        }
    }
}

// Tick the passed block
bool World::RandomTick(Block* b, Int3& pos) {
    switch(b->type) {
        case BLOCK_GRASS:
        {
            std::uniform_int_distribution<int> dist(-2,2);
            // Random offset
            pos = pos + Int3{dist(rng),dist(rng),dist(rng)};
            Block* nb = GetBlock(pos);
            if (nb && nb->type == BLOCK_DIRT) {
                Block* ab = GetBlock(pos+Int3{0,1,0});
                if (ab && ab->type == BLOCK_AIR) {
                    nb->type = BLOCK_GRASS;
                    return true;
                }
            }
            break;
        }
        case BLOCK_CROP_WHEAT:
        {
            if (b->meta < MAX_CROP_SIZE) {
                b->meta++;
            }
            return true;
        }
    }
    return false;
}

int8_t World::GetLight(bool skyLight, Int3 pos) {
    if(pos.y < 0) {
        return 15;
    } else if(pos.y >= 128) {
        return 15;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return 0;
        pos.x &= 15;
        pos.z &= 15;
        return c->GetLight(skyLight, pos);
    }
}

void World::SetLight(bool skyLight, Int3 pos, int8_t newLight) {
    if(pos.y < 0) {
        return;
    } else if(pos.y >= 128) {
        return;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return;
        pos.x &= 15;
        pos.z &= 15;
        return c->SetLight(skyLight, pos, newLight);
    }
}

void World::SetBlockType(int8_t blockType, Int3 pos) {
    if(pos.y < 0) {
        return;
    } else if(pos.y >= 128) {
        return;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return;
        pos.x &= 15;
        pos.z &= 15;
        return c->SetBlockType(blockType, pos);
    }
}

int8_t World::GetBlockType(Int3 pos) {
    if(pos.y < 0) {
        return 15;
    } else if(pos.y >= 128) {
        return 15;
    } else {
        Chunk* c = this->GetChunk(pos.x >> 4, pos.z >> 4);
        if (!c) return 0;
        pos.x &= 15;
        pos.z &= 15;
        return c->GetBlockType(pos);
    }
}

void World::AddTileEntity(std::unique_ptr<TileEntity>&& te) {
    Chunk* c = this->GetChunk(
        te->position.x >> 4,
        te->position.z >> 4
    );
    if (!c) return;
    c->AddTileEntity(std::move(te));
}