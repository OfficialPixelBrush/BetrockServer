#include "beta173Feature.h"

Beta173Feature::Beta173Feature(int16_t pId) {
    // Set the to-be-generated block
    this->id = pId;
}

Beta173Feature::Beta173Feature(int16_t pId, int8_t pMeta) {
    // Set the to-be-generated block
    this->id = pId;
    this->meta = pMeta;
}

// Generate a lake
bool Beta173Feature::GenerateLake(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    blockX -= 8;

    // Check for any non-air blocks
    for(blockZ -= 8; blockY > 0; --blockY){
        if (world->GetBlockType(Int3{blockX, blockY, blockZ}) != BLOCK_AIR)
            break;
    }

    blockY -= 4;
    bool shapeMask[2048] = {};
    int blobCount = rand->nextInt(4) + 4;

    for(int blobIndex = 0; blobIndex < blobCount; ++blobIndex) {
        double blobRadiusX = rand->nextDouble() * 6.0 + 3.0;
        double blobRadiusY = rand->nextDouble() * 4.0 + 2.0;
        double blobRadiusZ = rand->nextDouble() * 6.0 + 3.0;

        double blobCenterX = rand->nextDouble() * (16.0 - blobRadiusX - 2.0) + 1.0 + blobRadiusX / 2.0;
        double blobCenterY = rand->nextDouble() * (8.0 - blobRadiusY - 4.0) + 2.0 + blobRadiusY / 2.0;
        double blobCenterZ = rand->nextDouble() * (16.0 - blobRadiusZ - 2.0) + 1.0 + blobRadiusZ / 2.0;

        for(int x = 1; x < 15; ++x) {
            for(int z = 1; z < 15; ++z) {
                for(int y = 1; y < 7; ++y) {
                    double dx = ((double)x - blobCenterX) / (blobRadiusX / 2.0);
                    double dy = ((double)y - blobCenterY) / (blobRadiusY / 2.0);
                    double dz = ((double)z - blobCenterZ) / (blobRadiusZ / 2.0);
                    double distance = dx * dx + dy * dy + dz * dz;
                    if(distance < 1.0) {
                        shapeMask[(x * 16 + z) * 8 + y] = true;
                    }
                }
            }
        }
    }

    // Check if there's no other water nearby that'd intersect our new lake
    for(int x = 0; x < 16; ++x) {
        for(int z = 0; z < 16; ++z) {
            for(int y = 0; y < 8; ++y) {
                bool edge = (!shapeMask[(x * 16 + z) * 8 + y]) && 
                    (
                        ((x < 15)   && (shapeMask[((x + 1) * 16 + z) * 8 + y])) ||
                        ((x > 0)    && (shapeMask[((x - 1) * 16 + z) * 8 + y])) ||
                        ((z < 15)&& (shapeMask[(x * 16 + z + 1) * 8 + y]))   ||
                        ((z > 0) && (shapeMask[(x * 16 + (z - 1)) * 8 + y])) ||
                        ((y < 7)   && (shapeMask[(x * 16 + z) * 8 + y + 1]))   ||
                        ((y > 0)   && (shapeMask[(x * 16 + z) * 8 + (y - 1)]))
                    );
                if(edge) {
                    int8_t blockType = world->GetBlockType(Int3{blockX + x, blockY + y, blockZ + z});
                    if(y >= 4 && IsLiquid(blockType)) {
                        return false;
                    }
                    if(
                        y < 4 && !IsSolid(blockType) &&
                        world->GetBlockType(Int3{blockX + x, blockY + y, blockZ + z}) != this->id
                    ) {
                        return false;
                    }
                }
            }
        }
    }

    // Fill the lake
    for(int x = 0; x < 16; ++x) {
        for(int z = 0; z < 16; ++z) {
            for(int y = 0; y < 8; ++y) {
                if(shapeMask[(x * 16 + z) * 8 + y]) {
                    world->SetBlockType(
                        y >= 4 ? BLOCK_AIR : Blocks(this->id),
                        Int3{blockX + x, blockY + y, blockZ + z}
                    );
                }
            }
        }
    }

    // Replace exposed dirt with grass
    for(int x = 0; x < 16; ++x) {
        for(int z = 0; z < 16; ++z) {
            for(int y = 4; y < 8; ++y) {
                if(
                    shapeMask[(x * 16 + z) * 8 + y] &&
                    world->GetBlockType(Int3{blockX + x, blockY + y - 1, blockZ + z}) == BLOCK_DIRT &&
                    world->GetSkyLight( Int3{blockX + x, blockY + y, blockZ + z}) > 0
                ) {
                    world->SetBlockType(
                        BLOCK_GRASS,
                        Int3{blockX + x, blockY + y - 1, blockZ + z}
                    );
                }
            }
        }
    }

    // If we're generating a lava lake, make the edges into stone
    if(this->id == BLOCK_LAVA_STILL || this->id == BLOCK_LAVA_FLOWING) {
        for(int x = 0; x < 16; ++x) {
            for(int z = 0; z < 16; ++z) {
                for(int y = 0; y < 8; ++y) {
                    bool edge = !(shapeMask[(x * 16 + z) * 8 + y]) &&
                        (
                            (x < 15 && shapeMask[((x + 1) * 16 + z) * 8 + y]) ||
                            (x > 0 && shapeMask[((x - 1) * 16 + z) * 8 + y]) ||
                            (z < 15 && shapeMask[(x * 16 + z + 1) * 8 + y]) ||
                            (z > 0 && shapeMask[(x * 16 + (z - 1)) * 8 + y]) ||
                            (y < 7 && shapeMask[(x * 16 + z) * 8 + y + 1]) ||
                            (y > 0 && shapeMask[(x * 16 + z) * 8 + (y - 1)])
                        );
                    if(
                        edge &&
                        (y < 4 || rand->nextInt(2) != 0) &&
                        IsSolid(world->GetBlockType(Int3{blockX + x, blockY + y, blockZ + z}))
                    ) {
                        world->SetBlockType(BLOCK_STONE, Int3{blockX + x, blockY + y, blockZ + z});
                    }
                }
            }
        }
    }

    return true;
}

// Generate a dungeon
bool Beta173Feature::GenerateDungeon(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    int8_t dungeonHeight = 3;
    int dungeonWidthX = rand->nextInt(2) + 2;
    int dungeonWidthZ = rand->nextInt(2) + 2;
    int validEntires = 0;

    // Determine if a dungeon can be placed
    for(int xI = blockX - dungeonWidthX - 1; xI <= blockX + dungeonWidthX + 1; ++xI) {
        for(int yI = blockY - 1; yI <= blockY + dungeonHeight + 1; ++yI) {
            for(int zI = blockZ - dungeonWidthZ - 1; zI <= blockZ + dungeonWidthZ + 1; ++zI) {
                int8_t blockType = world->GetBlockType(Int3{xI, yI, zI});

                // Floor and ceiling must be solid
                if(yI == blockY - 1 && !IsSolid(blockType)) return false;
                if(yI == blockY + dungeonHeight + 1 && !IsSolid(blockType)) return false;

                if(
                    (
                        xI == blockX - dungeonWidthX - 1 ||
                        xI == blockX + dungeonWidthX + 1 ||
                        zI == blockZ - dungeonWidthZ - 1 ||
                        zI == blockZ + dungeonWidthZ + 1
                    ) && yI == blockY &&
                    blockType == BLOCK_AIR &&
                    world->GetBlockType(Int3{xI, yI + 1, zI}) == BLOCK_AIR
                ) {
                    ++validEntires;
                }
            }
        }
    }

    // There are too few or too many ways to get in, so we abort
    if(validEntires < 1 || validEntires > 5) {
        return false;
    }

    // Build the dungeon
    for(int xI = blockX - dungeonWidthX - 1; xI <= blockX + dungeonWidthX + 1; ++xI) {
        for(int yI = blockY + dungeonHeight; yI >= blockY - 1; --yI) {
            for(int zI = blockZ - dungeonWidthZ - 1; zI <= blockZ + dungeonWidthZ + 1; ++zI) {
                // Check if the current block is not a wall
                if(xI != blockX - dungeonWidthX - 1 &&
                    yI != blockY - 1 &&
                    zI != blockZ - dungeonWidthZ - 1 &&
                    xI != blockX + dungeonWidthX + 1 &&
                    yI != blockY + dungeonHeight + 1 &&
                    zI != blockZ + dungeonWidthZ + 1
                ) {
                    world->PlaceBlock(Int3{xI, yI, zI}, BLOCK_AIR);
                } else if(yI >= 0 && !IsSolid(world->GetBlockType(Int3{xI, yI - 1, zI}))) {
                    world->PlaceBlock(Int3{xI, yI, zI}, BLOCK_AIR);
                } else if(IsSolid(world->GetBlockType(Int3{xI, yI, zI}))) {
                    if(yI == blockY - 1 && rand->nextInt(4) != 0) {
                        world->PlaceBlock(Int3{xI, yI, zI}, BLOCK_COBBLESTONE_MOSSY);
                    } else {
                        world->PlaceBlock(Int3{xI, yI, zI}, BLOCK_COBBLESTONE);
                    }
                }
            }
        }
    }

    // Try placing up to 2 chests
    for(int chestAttempt = 0; chestAttempt < 2; ++chestAttempt) {
        for(int attempts = 0; attempts < 3; ++attempts) {
            int chestX = blockX + rand->nextInt(dungeonWidthX * 2 + 1) - dungeonWidthX;
            int chestZ = blockZ + rand->nextInt(dungeonWidthZ * 2 + 1) - dungeonWidthZ;

            if(world->GetBlockType(Int3{chestX, blockY, chestZ}) != BLOCK_AIR) continue;

            // Count the number of adjacent blocks
            int adjacentSolidBlocks = 0;
            if(IsSolid(world->GetBlockType(Int3{chestX - 1, blockY, chestZ}))) ++adjacentSolidBlocks;
            if(IsSolid(world->GetBlockType(Int3{chestX + 1, blockY, chestZ}))) ++adjacentSolidBlocks;
            if(IsSolid(world->GetBlockType(Int3{chestX, blockY, chestZ - 1}))) ++adjacentSolidBlocks;
            if(IsSolid(world->GetBlockType(Int3{chestX, blockY, chestZ + 1}))) ++adjacentSolidBlocks;

            // Only place a block if there's a single solid block
            if(adjacentSolidBlocks == 1) {
                Int3 chestLocation = Int3{chestX, blockY, chestZ};
                world->PlaceBlock(chestLocation, BLOCK_CHEST);
                std::unique_ptr<ChestTile> chest = std::make_unique<ChestTile>(chestLocation);

                for (int slotAttempt = 0; slotAttempt < 8; ++slotAttempt) {
                    Item item = GenerateDungeonChestLoot(rand);
                    if (item.id != SLOT_EMPTY) {
                        int slot = rand->nextInt(chest->GetInventory().size());
                        chest->SetSlot(slot, item);
                    }
                }
                world->AddTileEntity(std::move(chest));
                break;
            }
        }
    }

    Int3 mobSpawnerPos = Int3{blockX, blockY, blockZ};
    world->PlaceBlock(mobSpawnerPos, BLOCK_MOB_SPAWNER);
    world->AddTileEntity(std::make_unique<MobSpawnerTile>(mobSpawnerPos, PickMobToSpawn(rand)));
    return true;
}

// Generate Dungeon Chest loot
Item Beta173Feature::GenerateDungeonChestLoot(JavaRandom* rand) {
    int randValue = rand->nextInt(11);
    switch(randValue) {
        case 0:
            return Item{ITEM_SADDLE, 1, 0};
        case 1:
            return Item{ITEM_IRON, int8_t(rand->nextInt(4) + 1), 0};
        case 2:
            return Item{ITEM_BREAD, 1, 0};
        case 3:
            return Item{ITEM_WHEAT, int8_t(rand->nextInt(4) + 1), 0};
        case 4:
            return Item{ITEM_GUNPOWDER, int8_t(rand->nextInt(4) + 1), 0};
        case 5:
            return Item{ITEM_STRING, int8_t(rand->nextInt(4) + 1), 0};
        case 6:
            return Item{ITEM_BUCKET, 1, 0};
        case 7:
            if (rand->nextInt(100) == 0) return Item{ITEM_APPLE_GOLDEN, 1, 0};
            break;
        case 8:
            if (rand->nextInt(2) == 0) return Item{ITEM_REDSTONE, int8_t(rand->nextInt(4) + 1), 0};
            break;
        case 9:
            if (rand->nextInt(10) == 0) return Item{int16_t(ITEM_RECORD_13 + rand->nextInt(2)), 1, 0};
            break;
        case 10:
            return Item{ITEM_DYE, 1, 3};
    }
    return Item{SLOT_EMPTY,0,0};
}

// Pick a random monster that should be in a mob spawner
std::string Beta173Feature::PickMobToSpawn(JavaRandom* rand) {
    int mobIndex = rand->nextInt(4);
    switch(mobIndex) {
        case 0:
            return "Skeleton";
        case 1:
        case 2:
            return "Zombie";
        case 3:
            return "Spider";
    }
    return "";
}

// Generate a clay blob
bool Beta173Feature::GenerateClay(World* world, JavaRandom* rand, int xBlock, int yBlock, int zBlock, int blobSize) {
    // Clay can only generate around water
    int8_t blockType = world->GetBlockType(Int3{xBlock, yBlock, zBlock});
    if(blockType != BLOCK_WATER_STILL && blockType != BLOCK_WATER_FLOWING) {
        return false;
    }
    // Get angle of clay blob
    float angle = rand->nextFloat() * (float)M_PI;
    // Then determine the bounds of the blob
    double xStart = (double)((float)(xBlock + 8) + MathHelper::sin(angle) * (float)blobSize / 8.0F);
    double xEnd   = (double)((float)(xBlock + 8) - MathHelper::sin(angle) * (float)blobSize / 8.0F);
    double zStart = (double)((float)(zBlock + 8) + MathHelper::cos(angle) * (float)blobSize / 8.0F);
    double zEnd   = (double)((float)(zBlock + 8) - MathHelper::cos(angle) * (float)blobSize / 8.0F);
    double yStart = (double)(yBlock + rand->nextInt(3) + 2);
    double yEnd   = (double)(yBlock + rand->nextInt(3) + 2);

    // Interpolate between the start and end
    for(int i = 0; i <= blobSize; ++i) {
        double xCenter = xStart  + (xEnd    - xStart ) * (double)i / (double)blobSize;
        double yCenter = yStart  + (yEnd    - yStart ) * (double)i / (double)blobSize;
        double zCenter = zStart  + (zEnd    - zStart ) * (double)i / (double)blobSize;
        double blobScale = rand->nextDouble() * (double)blobSize / 16.0;
        double blobRadiusXZ = (double)(MathHelper::sin((float)i * (float)M_PI / (float)blobSize) + 1.0F) * blobScale  + 1.0;
        double blobRadiusY = (double)(MathHelper::sin((float)i * (float)M_PI / (float)blobSize) + 1.0F) * blobScale  + 1.0;
        int minX = int(MathHelper::floor_double(xCenter  - blobRadiusXZ  / 2.0));
        int maxX = int(MathHelper::floor_double(xCenter  + blobRadiusXZ  / 2.0));
        int minY = int(MathHelper::floor_double(yCenter  - blobRadiusY   / 2.0));
        int maxY = int(MathHelper::floor_double(yCenter  + blobRadiusY   / 2.0));
        int minZ = int(MathHelper::floor_double(zCenter  - blobRadiusXZ  / 2.0));
        int maxZ = int(MathHelper::floor_double(zCenter  + blobRadiusXZ  / 2.0));

        // Replace sand blocks in the relevant area
        for(int x = minX; x <= maxX; ++x) {
            for(int y = minY; y <= maxY; ++y) {
                for(int z = minZ; z <= maxZ; ++z) {
                    double dx = ((double)x + 0.5 - xCenter ) / (blobRadiusXZ  / 2.0);
                    double dy = ((double)y + 0.5 - yCenter ) / (blobRadiusY   / 2.0);
                    double dz = ((double)z + 0.5 - zCenter ) / (blobRadiusXZ  / 2.0);
                    if(dx * dx + dy * dy + dz * dz < 1.0) {
                        int currentBlock = world->GetBlockType(Int3{x, y, z});
                        if(currentBlock == BLOCK_SAND) {
                            world->PlaceBlock(Int3{x, y, z}, BLOCK_CLAY);
                        }
                    }
                }
            }
        }
    }

    return true;
}

// Generate a blob of ore or other material
bool Beta173Feature::GenerateMinable(World* world, JavaRandom* rand, int xBlock, int yBlock, int zBlock, int blobSize) {
    // Get angle of clay blob
    float angle = rand->nextFloat() * (float)M_PI;
    // Then determine the bounds of the blob
    double xStart = (double)((float)(xBlock + 8) + MathHelper::sin(angle) * (float)blobSize / 8.0F);
    double xEnd = (double)((float)(xBlock + 8) - MathHelper::sin(angle) * (float)blobSize / 8.0F);
    double zStart = (double)((float)(zBlock + 8) + MathHelper::cos(angle) * (float)blobSize / 8.0F);
    double zEnd = (double)((float)(zBlock + 8) - MathHelper::cos(angle) * (float)blobSize / 8.0F);
    double yStart = (double)(yBlock + rand->nextInt(3) + 2);
    double yEnd = (double)(yBlock + rand->nextInt(3) + 2);

    // Interpolate between the start and end
    for(int i = 0; i <= blobSize; ++i) {
        double xCenter = xStart  + (xEnd    - xStart ) * (double)i / (double)blobSize;
        double yCenter = yStart + (yEnd - yStart) * (double)i / (double)blobSize;
        double zCenter = zStart + (zEnd - zStart) * (double)i / (double)blobSize;
        double blobScale = rand->nextDouble() * (double)blobSize / 16.0;
        double blobRadiusXZ = (double)(MathHelper::sin((float)i * (float)M_PI / (float)blobSize) + 1.0F) * blobScale + 1.0;
        double blobRadiusY = (double)(MathHelper::sin((float)i * (float)M_PI / (float)blobSize) + 1.0F) * blobScale + 1.0;
        int minX = int(MathHelper::floor_double(xCenter - blobRadiusXZ / 2.0));
        int maxX = int(MathHelper::floor_double(yCenter - blobRadiusY / 2.0));
        int minY = int(MathHelper::floor_double(zCenter - blobRadiusXZ / 2.0));
        int maxY = int(MathHelper::floor_double(xCenter + blobRadiusXZ / 2.0));
        int minZ = int(MathHelper::floor_double(yCenter + blobRadiusY / 2.0));
        int maxZ = int(MathHelper::floor_double(zCenter + blobRadiusXZ / 2.0));

        // Replace stone blocks in the relevant area
        for(int x = minX; x <= maxY; ++x) {
            double dx = ((double)x + 0.5 - xCenter) / (blobRadiusXZ / 2.0);
            if(dx * dx < 1.0) {
                for(int z = maxX; z <= minZ; ++z) {
                    double dz = ((double)z + 0.5 - yCenter) / (blobRadiusY / 2.0);
                    if(dx * dx + dz * dz < 1.0) {
                        for(int y = minY; y <= maxZ; ++y) {
                            double dy = ((double)y + 0.5 - zCenter) / (blobRadiusXZ / 2.0);
                            if(
                                dx * dx + dz * dz + dy * dy < 1.0 &&
                                world->GetBlockType(Int3{x, z, y}) == BLOCK_STONE
                            ) {
                                world->SetBlockType(int8_t(this->id), Int3{x, z, y});
                            }
                        }
                    }
                }
            }
        }
    }

    return true;
}

bool Beta173Feature::GenerateFlowers(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    for(int i = 0; i < CHUNK_HEIGHT/2; ++i) {
        int offsetX = blockX + rand->nextInt(8) - rand->nextInt(8);
        int offsetY = blockY + rand->nextInt(4) - rand->nextInt(4);
        int offsetZ = blockZ + rand->nextInt(8) - rand->nextInt(8);
        if(
            world->GetBlockType(Int3{offsetX, offsetY, offsetZ}) == BLOCK_AIR &&
            CanStay(this->id, world, Int3{offsetX, offsetY, offsetZ})
        ) {
            world->SetBlockType(this->id, Int3{offsetX, offsetY, offsetZ});
        }
    }

    return true;
}

bool Beta173Feature::GenerateTallgrass(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    while(true) {
        int blockType = world->GetBlockType(Int3{blockX, blockY, blockZ});
        if((blockType != 0 && blockType != BLOCK_LEAVES) || blockY <= 0) {
            for(int y = 0; y < CHUNK_HEIGHT; ++y) {
                int offsetX = blockX + rand->nextInt(8) - rand->nextInt(8);
                int offsetY = blockY + rand->nextInt(4) - rand->nextInt(4);
                int offsetZ = blockZ + rand->nextInt(8) - rand->nextInt(8);
                if(
                    world->GetBlockType(Int3{offsetX, offsetY, offsetZ}) == BLOCK_AIR &&
                    CanStay(this->id, world, Int3{offsetX, offsetY, offsetZ})
                ) {
                    world->SetBlockTypeAndMeta(this->id, this->meta, Int3{offsetX, offsetY, offsetZ});
                }
            }

            return true;
        }

        --blockY;
    }
}

bool Beta173Feature::GenerateDeadbush(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    while(true) {
        int blockType = world->GetBlockType(Int3{blockX, blockY, blockZ});
        if((blockType != 0 && blockType != BLOCK_LEAVES) || blockY <= 0) {
            for(int i = 0; i < 4; ++i) {
                int offsetX = blockX + rand->nextInt(8) - rand->nextInt(8);
                int offsetY = blockY + rand->nextInt(4) - rand->nextInt(4);
                int offsetZ = blockZ + rand->nextInt(8) - rand->nextInt(8);
                if(
                    world->GetBlockType(Int3{offsetX, offsetY, offsetZ}) == BLOCK_AIR &&
                    CanStay(this->id, world, Int3{offsetX, offsetY, offsetZ}))
                {
                    world->SetBlockType(this->id, Int3{offsetX, offsetY, offsetZ});
                }
            }

            return true;
        }

        --blockY;
    }
}

bool Beta173Feature::GenerateSugarcane(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    for(int i = 0; i < 20; ++i) {
        int xOffset = blockX + rand->nextInt(4) - rand->nextInt(4);
        int zOffset = blockZ + rand->nextInt(4) - rand->nextInt(4);
        if(
            world->GetBlockType(Int3{xOffset, blockY, zOffset}) == BLOCK_AIR &&
            (
                world->GetBlockType(Int3{xOffset - 1, blockY - 1, zOffset    }) == BLOCK_WATER_STILL ||
                world->GetBlockType(Int3{xOffset - 1, blockY - 1, zOffset    }) == BLOCK_WATER_FLOWING ||
                world->GetBlockType(Int3{xOffset + 1, blockY - 1, zOffset    }) == BLOCK_WATER_STILL ||
                world->GetBlockType(Int3{xOffset + 1, blockY - 1, zOffset    }) == BLOCK_WATER_FLOWING ||
                world->GetBlockType(Int3{xOffset    , blockY - 1, zOffset - 1}) == BLOCK_WATER_STILL ||
                world->GetBlockType(Int3{xOffset    , blockY - 1, zOffset - 1}) == BLOCK_WATER_FLOWING ||
                world->GetBlockType(Int3{xOffset    , blockY - 1, zOffset + 1}) == BLOCK_WATER_STILL ||
                world->GetBlockType(Int3{xOffset    , blockY - 1, zOffset + 1}) == BLOCK_WATER_FLOWING
            )
        ) {
            int height = 2 + rand->nextInt(rand->nextInt(3) + 1);

            for(int h = 0; h < height; ++h) {
                if(CanStay(BLOCK_SUGARCANE, world, Int3{xOffset, blockY + h, zOffset})) {
                    world->SetBlockType(BLOCK_SUGARCANE, Int3{xOffset, blockY + h, zOffset});
                }
            }
        }
    }

    return true;
}

bool Beta173Feature::GeneratePumpkins(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    for(int i = 0; i < 64; ++i) {
        int xOffset = blockX + rand->nextInt(8) - rand->nextInt(8);
        int yOffset = blockY + rand->nextInt(4) - rand->nextInt(4);
        int zOffset = blockZ + rand->nextInt(8) - rand->nextInt(8);
        if(
            world->GetBlockType(Int3{xOffset, yOffset, zOffset}) == BLOCK_AIR &&
            world->GetBlockType(Int3{xOffset, yOffset - 1, zOffset}) == BLOCK_GRASS &&
            CanBePlaced(BLOCK_PUMPKIN, world, Int3{xOffset, yOffset, zOffset})
        ) {
            world->SetBlockTypeAndMeta(BLOCK_PUMPKIN, rand->nextInt(4), Int3{xOffset, yOffset, zOffset});
        }
    }
    return true;
}


bool Beta173Feature::GenerateCacti(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    for(int i = 0; i < 10; ++i) {
        int xOffset = blockX + rand->nextInt(8) - rand->nextInt(8);
        int yOffset = blockY + rand->nextInt(4) - rand->nextInt(4);
        int zOffset = blockZ + rand->nextInt(8) - rand->nextInt(8);
        if(world->GetBlockType(Int3{xOffset, yOffset, zOffset}) == BLOCK_AIR) {
            int height = 1 + rand->nextInt(rand->nextInt(3) + 1);

            for(int h = 0; h < height; ++h) {
                if(CanStay(BLOCK_CACTUS, world, Int3{xOffset, yOffset + h, zOffset})) {
                    world->SetBlockType(BLOCK_CACTUS, Int3{xOffset, yOffset + h, zOffset});
                }
            }
        }
    }

    return true;
}

bool Beta173Feature::GenerateLiquid(World* world, [[maybe_unused]] JavaRandom* rand, int blockX, int blockY, int blockZ) {
    if(world->GetBlockType(Int3{blockX, blockY + 1, blockZ}) != BLOCK_STONE) {
        return false;
    } else if(world->GetBlockType(Int3{blockX, blockY - 1, blockZ}) != BLOCK_STONE) {
        return false;
    } else if(
        world->GetBlockType(Int3{blockX, blockY, blockZ}) != BLOCK_AIR &&
        world->GetBlockType(Int3{blockX, blockY, blockZ}) != BLOCK_STONE
    ) {
        return false;
    } else {
        int surroundingStone = 0;
        if(world->GetBlockType(Int3{blockX - 1, blockY, blockZ}) == BLOCK_STONE) ++surroundingStone;
        if(world->GetBlockType(Int3{blockX + 1, blockY, blockZ}) == BLOCK_STONE) ++surroundingStone;
        if(world->GetBlockType(Int3{blockX, blockY, blockZ - 1}) == BLOCK_STONE) ++surroundingStone;
        if(world->GetBlockType(Int3{blockX, blockY, blockZ + 1}) == BLOCK_STONE) ++surroundingStone;

        int surroundingAir = 0;
        if(world->GetBlockType(Int3{blockX - 1, blockY, blockZ}) == BLOCK_AIR) ++surroundingAir;
        if(world->GetBlockType(Int3{blockX + 1, blockY, blockZ}) == BLOCK_AIR) ++surroundingAir;
        if(world->GetBlockType(Int3{blockX, blockY, blockZ - 1}) == BLOCK_AIR) ++surroundingAir;
        if(world->GetBlockType(Int3{blockX, blockY, blockZ + 1}) == BLOCK_AIR) ++surroundingAir;

        if(surroundingStone == 3 && surroundingAir == 1) {
            world->SetBlockType(this->id, Int3{blockX, blockY, blockZ});
            //var1.scheduledUpdatesAreImmediate = true;
            //Block.blocksList[this.liquidBlockId].updateTick(var1, var3, var4, var5, var2);
            //var1.scheduledUpdatesAreImmediate = false;
        }

        return true;
    }
}