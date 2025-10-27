#include "beta173Feature.h"

Beta173Feature::Beta173Feature(int16_t id) {
    // Set the to-be-generated block
    this->id = id;
}

bool Beta173Feature::GenerateLake(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    blockX -= 8;

    for(blockZ -= 8; blockY > 0; --blockY){
        Block* b = world->GetBlock(Int3{blockX, blockY, blockZ});
        if (!b) continue;
        if (b->type != BLOCK_AIR) break;
    }

    blockY -= 4;
    bool yOffset[2048];
    int xOffset = rand->nextInt(4) + 4;

    int zOffset;
    for(zOffset = 0; zOffset < xOffset; ++zOffset) {
        double var9 = rand->nextDouble() * 6.0D + 3.0D;
        double yI = rand->nextDouble() * 4.0D + 2.0D;
        double var13 = rand->nextDouble() * 6.0D + 3.0D;
        double var15 = rand->nextDouble() * (16.0D - var9 - 2.0D) + 1.0D + var9 / 2.0D;
        double numberOfItems = rand->nextDouble() * (8.0D - yI - 4.0D) + 2.0D + yI / 2.0D;
        double var19 = rand->nextDouble() * (16.0D - var13 - 2.0D) + 1.0D + var13 / 2.0D;

        for(int var21 = 1; var21 < 15; ++var21) {
            for(int var22 = 1; var22 < 15; ++var22) {
                for(int var23 = 1; var23 < 7; ++var23) {
                    double var24 = ((double)var21 - var15) / (var9 / 2.0D);
                    double var26 = ((double)var23 - numberOfItems) / (yI / 2.0D);
                    double var28 = ((double)var22 - var19) / (var13 / 2.0D);
                    double blockX0 = var24 * var24 + var26 * var26 + var28 * var28;
                    if(blockX0 < 1.0D) {
                        yOffset[(var21 * 16 + var22) * 8 + var23] = true;
                    }
                }
            }
        }
    }

    int xI;
    int blockX2;
    bool blockX3;
    for(zOffset = 0; zOffset < 16; ++zOffset) {
        for(blockX2 = 0; blockX2 < 16; ++blockX2) {
            for(xI = 0; xI < 8; ++xI) {
                blockX3 = (!yOffset[(zOffset * 16 + blockX2) * 8 + xI]) && 
                    (
                        ((zOffset < 15)   && (yOffset[((zOffset + 1) * 16 + blockX2) * 8 + xI])) ||
                        ((zOffset > 0)    && (yOffset[((zOffset - 1) * 16 + blockX2) * 8 + xI])) ||
                        ((blockX2 < 15)&& (yOffset[(zOffset * 16 + blockX2 + 1) * 8 + xI]))   ||
                        ((blockX2 > 0) && (yOffset[(zOffset * 16 + (blockX2 - 1)) * 8 + xI])) ||
                        ((xI < 7)   && (yOffset[(zOffset * 16 + blockX2) * 8 + xI + 1]))   ||
                        ((xI > 0)   && (yOffset[(zOffset * 16 + blockX2) * 8 + (xI - 1)]))
                    );
                if(blockX3) {
                    Block* b = world->GetBlock(Int3{blockX + zOffset, blockY + xI, blockZ + blockX2});
                    if (!b) continue;
                    if(xI >= 4 && IsLiquid(b->type)) {
                        return false;
                    }

                    if(xI < 4 && !IsSolid(b->type) && world->GetBlockType(Int3{blockX + zOffset, blockY + xI, blockZ + blockX2}) != this->id) {
                        return false;
                    }
                }
            }
        }
    }

    for(zOffset = 0; zOffset < 16; ++zOffset) {
        for(blockX2 = 0; blockX2 < 16; ++blockX2) {
            for(xI = 0; xI < 8; ++xI) {
                if(yOffset[(zOffset * 16 + blockX2) * 8 + xI]) {
                    world->SetBlockType(xI >= 4 ? 0 : this->id, Int3{blockX + zOffset, blockY + xI, blockZ + blockX2});
                }
            }
        }
    }

    for(zOffset = 0; zOffset < 16; ++zOffset) {
        for(blockX2 = 0; blockX2 < 16; ++blockX2) {
            for(xI = 4; xI < 8; ++xI) {
                Block* b = world->GetBlock(Int3{blockX + zOffset, blockY + xI - 1, blockZ + blockX2});
                if (!b) continue;
                if(
                    yOffset[(zOffset * 16 + blockX2) * 8 + xI] &&
                    b->type == BLOCK_DIRT &&
                    world->GetSkyLight( Int3{blockX + zOffset, blockY + xI, blockZ + blockX2}) > 0
                ) {
                    world->SetBlockType(BLOCK_GRASS, Int3{blockX + zOffset, blockY + xI - 1, blockZ + blockX2});
                }
            }
        }
    }

    if(this->id == BLOCK_LAVA_STILL || this->id == BLOCK_LAVA_FLOWING) {
        for(zOffset = 0; zOffset < 16; ++zOffset) {
            for(blockX2 = 0; blockX2 < 16; ++blockX2) {
                for(xI = 0; xI < 8; ++xI) {
                    blockX3 = 
                        !(yOffset[(zOffset * 16 + blockX2) * 8 + xI]) &&
                        (
                            (zOffset < 15 && yOffset[((zOffset + 1) * 16 + blockX2) * 8 + xI]) ||
                            (zOffset > 0 && yOffset[((zOffset - 1) * 16 + blockX2) * 8 + xI]) ||
                            (blockX2 < 15 && yOffset[(zOffset * 16 + blockX2 + 1) * 8 + xI]) ||
                            (blockX2 > 0 && yOffset[(zOffset * 16 + (blockX2 - 1)) * 8 + xI]) ||
                            (xI < 7 && yOffset[(zOffset * 16 + blockX2) * 8 + xI + 1]) ||
                            (xI > 0 && yOffset[(zOffset * 16 + blockX2) * 8 + (xI - 1)])
                        );
                    Block* b = world->GetBlock(Int3{blockX + zOffset, blockY + xI, blockZ + blockX2});
                    if (!b) continue;
                    if(blockX3 && (xI < 4 || rand->nextInt(2) != 0) && IsSolid(b->type)) {
                        world->SetBlockType(BLOCK_STONE, Int3{blockX + zOffset, blockY + xI, blockZ + blockX2});
                    }
                }
            }
        }
    }

    return true;
}

bool Beta173Feature::GenerateDungeon(World* world, JavaRandom* rand, int blockX, int blockY, int blockZ) {
    int8_t yOffset = 3;
    int xOffset = rand->nextInt(2) + 2;
    int zOffset = rand->nextInt(2) + 2;
    int var9 = 0;

    int xI;
    int yI;
    int zI;
    for(xI = blockX - xOffset - 1; xI <= blockX + xOffset + 1; ++xI) {
        for(yI = blockY - 1; yI <= blockY + yOffset + 1; ++yI) {
            for(zI = blockZ - zOffset - 1; zI <= blockZ + zOffset + 1; ++zI) {
                Block* b = world->GetBlock(Int3{xI, yI, zI});
                if (!b) continue;
                if(yI == blockY - 1 && !IsSolid(b->type)) {
                    return false;
                }

                if(yI == blockY + yOffset + 1 && !IsSolid(b->type)) {
                    return false;
                }

                if(
                    (
                        xI == blockX - xOffset - 1 ||
                        xI == blockX + xOffset + 1 ||
                        zI == blockZ - zOffset - 1 ||
                        zI == blockZ + zOffset + 1
                    ) && yI == blockY &&
                    b->type == BLOCK_AIR &&
                    world->GetBlockType(Int3{xI, yI + 1, zI}) == BLOCK_AIR
                ) {
                    ++var9;
                }
            }
        }
    }

    if(var9 >= 1 && var9 <= 5) {
        for(xI = blockX - xOffset - 1; xI <= blockX + xOffset + 1; ++xI) {
            for(yI = blockY + yOffset; yI >= blockY - 1; --yI) {
                for(zI = blockZ - zOffset - 1; zI <= blockZ + zOffset + 1; ++zI) {
                    if(xI != blockX - xOffset - 1 && yI != blockY - 1 && zI != blockZ - zOffset - 1 && xI != blockX + xOffset + 1 && yI != blockY + yOffset + 1 && zI != blockZ + zOffset + 1) {
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

        for(xI = 0; xI < 2; ++xI) {
            for(yI = 0; yI < 3; ++yI) {
                zI = blockX + rand->nextInt(xOffset * 2 + 1) - xOffset;
                int var14 = blockZ + rand->nextInt(zOffset * 2 + 1) - zOffset;
                if(world->GetBlockType(Int3{zI, blockY, var14}) == BLOCK_AIR) {
                    int var15 = 0;
                    if(IsSolid(world->GetBlockType(Int3{zI - 1, blockY, var14}))) {
                        ++var15;
                    }

                    if(IsSolid(world->GetBlockType(Int3{zI + 1, blockY, var14}))) {
                        ++var15;
                    }

                    if(IsSolid(world->GetBlockType(Int3{zI, blockY, var14 - 1}))) {
                        ++var15;
                    }

                    if(IsSolid(world->GetBlockType(Int3{zI, blockY, var14 + 1}))) {
                        ++var15;
                    }

                    if(var15 == 1) {
                        Int3 chestLocation = Int3{zI, blockY, var14};
                        world->PlaceBlock(chestLocation, BLOCK_CHEST);
                        world->AddTileEntity(std::make_unique<ChestTile>(chestLocation));
                        int numberOfItems = 0;

                        while(true) {
                            if(numberOfItems >= 8) {
                                break;
                            }

                            Item item = GenerateDungeonChestLoot(rand);
                            if(item.id != SLOT_EMPTY) {
                                TileEntity* te = world->GetTileEntity(chestLocation);
                                if (!te) continue;
                                ChestTile* ct = dynamic_cast<ChestTile*>(te);
                                if (!ct) continue;
                                ct->SetSlot(rand->nextInt(ct->GetInventory().size()), item);
                            }

                            ++numberOfItems;
                        }
                    }
                }
            }
        }

        Int3 mobSpawnerPos = Int3{blockX, blockY, blockZ};
        world->PlaceBlock(mobSpawnerPos, BLOCK_MOB_SPAWNER);
        world->AddTileEntity(std::make_unique<MobSpawnerTile>(mobSpawnerPos, PickMobToSpawn(rand)));
        return true;
    } else {
        return false;
    }
    return false;
}

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

bool Beta173Feature::GenerateClay(World* world, JavaRandom* rand, int xBlock, int yBlock, int zBlock, int numberOfBlocks) {
    int8_t blockType = world->GetBlockType(Int3{xBlock, yBlock, zBlock});
    if(blockType != BLOCK_WATER_STILL && blockType != BLOCK_WATER_FLOWING) {
        return false;
    }
    float var6 = rand->nextFloat() * (float)M_PI;
    double var7 = (double)((float)(xBlock + 8) + std::sin(var6) * (float)numberOfBlocks / 8.0F);
    double var9 = (double)((float)(xBlock + 8) - std::sin(var6) * (float)numberOfBlocks / 8.0F);
    double world1 = (double)((float)(zBlock + 8) + std::cos(var6) * (float)numberOfBlocks / 8.0F);
    double world3 = (double)((float)(zBlock + 8) - std::cos(var6) * (float)numberOfBlocks / 8.0F);
    double world5 = (double)(yBlock + rand->nextInt(3) + 2);
    double world7 = (double)(yBlock + rand->nextInt(3) + 2);

    for(int world9 = 0; world9 <= numberOfBlocks; ++world9) {
        double rand0 = var7 + (var9 - var7) * (double)world9 / (double)numberOfBlocks;
        double rand2 = world5 + (world7 - world5) * (double)world9 / (double)numberOfBlocks;
        double rand4 = world1 + (world3 - world1) * (double)world9 / (double)numberOfBlocks;
        double rand6 = rand->nextDouble() * (double)numberOfBlocks / 16.0D;
        double rand8 = (double)(std::sin((float)world9 * (float)M_PI / (float)numberOfBlocks) + 1.0F) * rand6 + 1.0D;
        double xBlock0 = (double)(std::sin((float)world9 * (float)M_PI / (float)numberOfBlocks) + 1.0F) * rand6 + 1.0D;
        int xBlock2 = int(std::floor(rand0 - rand8 / 2.0D));
        int xBlock3 = int(std::floor(rand0 + rand8 / 2.0D));
        int xBlock4 = int(std::floor(rand2 - xBlock0 / 2.0D));
        int xBlock5 = int(std::floor(rand2 + xBlock0 / 2.0D));
        int xBlock6 = int(std::floor(rand4 - rand8 / 2.0D));
        int xBlock7 = int(std::floor(rand4 + rand8 / 2.0D));

        for(int xBlock8 = xBlock2; xBlock8 <= xBlock3; ++xBlock8) {
            for(int xBlock9 = xBlock4; xBlock9 <= xBlock5; ++xBlock9) {
                for(int yBlock0 = xBlock6; yBlock0 <= xBlock7; ++yBlock0) {
                    double yBlock1 = ((double)xBlock8 + 0.5D - rand0) / (rand8 / 2.0D);
                    double yBlock3 = ((double)xBlock9 + 0.5D - rand2) / (xBlock0 / 2.0D);
                    double yBlock5 = ((double)yBlock0 + 0.5D - rand4) / (rand8 / 2.0D);
                    if(yBlock1 * yBlock1 + yBlock3 * yBlock3 + yBlock5 * yBlock5 < 1.0D) {
                        int yBlock7 = world->GetBlockType(Int3{xBlock8, xBlock9, yBlock0});
                        if(yBlock7 == BLOCK_SAND) {
                            world->PlaceBlock(Int3{xBlock8, xBlock9, yBlock0}, BLOCK_CLAY);
                        }
                    }
                }
            }
        }
    }

    return true;
}