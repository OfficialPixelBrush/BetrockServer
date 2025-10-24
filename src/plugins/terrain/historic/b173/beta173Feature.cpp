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
    bool var6[2048];
    int var7 = rand->nextInt(4) + 4;

    int var8;
    for(var8 = 0; var8 < var7; ++var8) {
        double var9 = rand->nextDouble() * 6.0D + 3.0D;
        double var11 = rand->nextDouble() * 4.0D + 2.0D;
        double var13 = rand->nextDouble() * 6.0D + 3.0D;
        double var15 = rand->nextDouble() * (16.0D - var9 - 2.0D) + 1.0D + var9 / 2.0D;
        double var17 = rand->nextDouble() * (8.0D - var11 - 4.0D) + 2.0D + var11 / 2.0D;
        double var19 = rand->nextDouble() * (16.0D - var13 - 2.0D) + 1.0D + var13 / 2.0D;

        for(int var21 = 1; var21 < 15; ++var21) {
            for(int var22 = 1; var22 < 15; ++var22) {
                for(int var23 = 1; var23 < 7; ++var23) {
                    double var24 = ((double)var21 - var15) / (var9 / 2.0D);
                    double var26 = ((double)var23 - var17) / (var11 / 2.0D);
                    double var28 = ((double)var22 - var19) / (var13 / 2.0D);
                    double blockX0 = var24 * var24 + var26 * var26 + var28 * var28;
                    if(blockX0 < 1.0D) {
                        var6[(var21 * 16 + var22) * 8 + var23] = true;
                    }
                }
            }
        }
    }

    int var10;
    int blockX2;
    bool blockX3;
    for(var8 = 0; var8 < 16; ++var8) {
        for(blockX2 = 0; blockX2 < 16; ++blockX2) {
            for(var10 = 0; var10 < 8; ++var10) {
                blockX3 = (!var6[(var8 * 16 + blockX2) * 8 + var10]) && 
                    (
                        ((var8 < 15)   && (var6[((var8 + 1) * 16 + blockX2) * 8 + var10])) ||
                        ((var8 > 0)    && (var6[((var8 - 1) * 16 + blockX2) * 8 + var10])) ||
                        ((blockX2 < 15)&& (var6[(var8 * 16 + blockX2 + 1) * 8 + var10]))   ||
                        ((blockX2 > 0) && (var6[(var8 * 16 + (blockX2 - 1)) * 8 + var10])) ||
                        ((var10 < 7)   && (var6[(var8 * 16 + blockX2) * 8 + var10 + 1]))   ||
                        ((var10 > 0)   && (var6[(var8 * 16 + blockX2) * 8 + (var10 - 1)]))
                    );
                if(blockX3) {
                    //Material var12 = world->getBlockMaterial(blockX + var8, blockY + var10, blockZ + blockX2);
                    Block* b = world->GetBlock(Int3{blockX + var8, blockY + var10, blockZ + blockX2});
                    if (!b) continue;
                    if(var10 >= 4 && IsLiquid(b->type)) {
                        return false;
                    }

                    if(var10 < 4 && !IsSolid(b->type) && world->GetBlockType(Int3{blockX + var8, blockY + var10, blockZ + blockX2}) != this->id) {
                        return false;
                    }
                }
            }
        }
    }

    for(var8 = 0; var8 < 16; ++var8) {
        for(blockX2 = 0; blockX2 < 16; ++blockX2) {
            for(var10 = 0; var10 < 8; ++var10) {
                if(var6[(var8 * 16 + blockX2) * 8 + var10]) {
                    world->SetBlockType(var10 >= 4 ? 0 : this->id, Int3{blockX + var8, blockY + var10, blockZ + blockX2});
                }
            }
        }
    }

    for(var8 = 0; var8 < 16; ++var8) {
        for(blockX2 = 0; blockX2 < 16; ++blockX2) {
            for(var10 = 4; var10 < 8; ++var10) {
                Block* b = world->GetBlock(Int3{blockX + var8, blockY + var10 - 1, blockZ + blockX2});
                if (!b) continue;
                if(
                    var6[(var8 * 16 + blockX2) * 8 + var10] &&
                    b->type == BLOCK_DIRT &&
                    world->GetSkyLight( Int3{blockX + var8, blockY + var10, blockZ + blockX2}) > 0
                ) {
                    world->SetBlockType(BLOCK_GRASS, Int3{blockX + var8, blockY + var10 - 1, blockZ + blockX2});
                }
            }
        }
    }

    if(this->id == BLOCK_LAVA_STILL || this->id == BLOCK_LAVA_FLOWING) {
        for(var8 = 0; var8 < 16; ++var8) {
            for(blockX2 = 0; blockX2 < 16; ++blockX2) {
                for(var10 = 0; var10 < 8; ++var10) {
                    blockX3 = 
                        !(var6[(var8 * 16 + blockX2) * 8 + var10]) &&
                        (
                            (var8 < 15 && var6[((var8 + 1) * 16 + blockX2) * 8 + var10]) ||
                            (var8 > 0 && var6[((var8 - 1) * 16 + blockX2) * 8 + var10]) ||
                            (blockX2 < 15 && var6[(var8 * 16 + blockX2 + 1) * 8 + var10]) ||
                            (blockX2 > 0 && var6[(var8 * 16 + (blockX2 - 1)) * 8 + var10]) ||
                            (var10 < 7 && var6[(var8 * 16 + blockX2) * 8 + var10 + 1]) ||
                            (var10 > 0 && var6[(var8 * 16 + blockX2) * 8 + (var10 - 1)])
                        );
                    Block* b = world->GetBlock(Int3{blockX + var8, blockY + var10, blockZ + blockX2});
                    if (!b) continue;
                    if(blockX3 && (var10 < 4 || rand->nextInt(2) != 0) && IsSolid(b->type)) {
                        world->SetBlockType(BLOCK_STONE, Int3{blockX + var8, blockY + var10, blockZ + blockX2});
                    }
                }
            }
        }
    }

    return true;
}