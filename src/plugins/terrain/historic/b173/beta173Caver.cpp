#include "beta173Caver.h"
Beta173Caver::Beta173Caver() {
    rand = std::make_unique<JavaRandom>();
}

void Beta173Caver::GenerateCaves(World* world, int cX, int cZ, std::unique_ptr<Chunk>& c) {
    int genOffset = this->generatorOffset;
    this->rand->setSeed(world->seed);
    long xOffset = this->rand->nextLong() / 2L * 2L + 1L;
    long zOffset = this->rand->nextLong() / 2L * 2L + 1L;

    for(int x = cX - genOffset; x <= cX + genOffset; ++x) {
        for(int z = cZ - genOffset; z <= cZ + genOffset; ++z) {
            this->rand->setSeed((long)x * xOffset + (long)z * zOffset ^ world->seed);
            this->GenerateCave(world, x, z, cX, cZ, c);
        }
    }
}

// TODO: This is only the cave generator for the overworld.
// The one for the nether is different!
void Beta173Caver::GenerateCave(World* world, int x, int z, int cX, int cZ, std::unique_ptr<Chunk>& c) {
    int var7 = this->rand->nextInt(this->rand->nextInt(this->rand->nextInt(40) + 1) + 1);
    if(this->rand->nextInt(15) != 0) {
        var7 = 0;
    }

    for(int var8 = 0; var8 < var7; ++var8) {
        double xOffset = (double)(x * CHUNK_WIDTH_X + this->rand->nextInt(CHUNK_WIDTH_X));
        double yOffset = (double)this->rand->nextInt(this->rand->nextInt(120) + 8);
        double zOffset = (double)(z * CHUNK_WIDTH_Z + this->rand->nextInt(CHUNK_WIDTH_Z));
        int var15 = 1;
        if(this->rand->nextInt(4) == 0) {
            this->CarveCave(cX, cZ, c, xOffset, yOffset, zOffset);
            var15 += this->rand->nextInt(4);
        }

        for(int var16 = 0; var16 < var15; ++var16) {
            float var17 = this->rand->nextFloat() * (float)M_PI * 2.0F;
            float var18 = (this->rand->nextFloat() - 0.5F) * 2.0F / 8.0F;
            float var19 = this->rand->nextFloat() * 2.0F + this->rand->nextFloat();
            this->CarveCave(cX, cZ, c, xOffset, yOffset, zOffset, var19, var17, var18, 0, 0, 1.0D);
        }
    }
}

void Beta173Caver::CarveCave(int cX, int cZ, std::unique_ptr<Chunk>& c, double xOffset, double yOffset, double zOffset) {
    this->CarveCave(cX, cZ, c, xOffset, yOffset, zOffset, 1.0F + this->rand->nextFloat() * 6.0F, 0.0F, 0.0F, -1, -1, 0.5D);
}

void Beta173Caver::CarveCave(int cX, int cZ, std::unique_ptr<Chunk>& c, double xOffset, double yOffset, double zOffset, float var10, float var11, float var12, int var13, int var14, double var15) {
    double var17 = (double)(cX * CHUNK_WIDTH_X + 8);
    double var19 = (double)(cZ * CHUNK_WIDTH_Z + 8);
    float var21 = 0.0F;
    float var22 = 0.0F;
    std::unique_ptr<JavaRandom> var23 = std::make_unique<JavaRandom>(this->rand->nextLong());
    if(var14 <= 0) {
        int var24 = this->generatorOffset * 16 - 16;
        var14 = var24 - var23->nextInt(var24 / 4);
    }

    bool var52 = false;
    if(var13 == -1) {
        var13 = var14 / 2;
        var52 = true;
    }

    int var25 = var23->nextInt(var14 / 2) + var14 / 4;

    for(bool var26 = var23->nextInt(6) == 0; var13 < var14; ++var13) {
        double var27 = 1.5D + (double)(std::sin((float)var13 * (float)M_PI / (float)var14) * var10 * 1.0F);
        double var29 = var27 * var15;
        float var31 = std::cos(var12);
        float var32 = std::sin(var12);
        xOffset += (double)(std::cos(var11) * var31);
        yOffset += (double)var32;
        zOffset += (double)(std::sin(var11) * var31);
        if(var26) {
            var12 *= 0.92F;
        } else {
            var12 *= 0.7F;
        }

        var12 += var22 * 0.1F;
        var11 += var21 * 0.1F;
        var22 *= 0.9F;
        var21 *= 12.0F / 16.0F;
        var22 += (var23->nextFloat() - var23->nextFloat()) * var23->nextFloat() * 2.0F;
        var21 += (var23->nextFloat() - var23->nextFloat()) * var23->nextFloat() * 4.0F;
        if(!var52 && var13 == var25 && var10 > 1.0F) {
            this->CarveCave(cX, cZ, c, xOffset, yOffset, zOffset, var23->nextFloat() * 0.5F + 0.5F, var11 - (float)M_PI * 0.5F, var12 / 3.0F, var13, var14, 1.0D);
            this->CarveCave(cX, cZ, c, xOffset, yOffset, zOffset, var23->nextFloat() * 0.5F + 0.5F, var11 + (float)M_PI * 0.5F, var12 / 3.0F, var13, var14, 1.0D);
            return;
        }

        if(var52 || var23->nextInt(4) != 0) {
            double var33 = xOffset - var17;
            double var35 = zOffset - var19;
            double var37 = (double)(var14 - var13);
            double var39 = (double)(var10 + 2.0F + 16.0F);
            if(var33 * var33 + var35 * var35 - var37 * var37 > var39 * var39) {
                return;
            }

            if(xOffset >= var17 - 16.0D - var27 * 2.0D && zOffset >= var19 - 16.0D - var27 * 2.0D && xOffset <= var17 + 16.0D + var27 * 2.0D && zOffset <= var19 + 16.0D + var27 * 2.0D) {
                int var53 = floor(xOffset - var27) - cX * 16 - 1;
                int var34 = floor(xOffset + var27) - cX * 16 + 1;
                int var54 = floor(yOffset - var29) - 1;
                int var36 = floor(yOffset + var29) + 1;
                int var55 = floor(zOffset - var27) - cZ * 16 - 1;
                int var38 = floor(zOffset + var27) - cZ * 16 + 1;
                if(var53 < 0) {
                    var53 = 0;
                }

                if(var34 > 16) {
                    var34 = 16;
                }

                if(var54 < 1) {
                    var54 = 1;
                }

                if(var36 > 120) {
                    var36 = 120;
                }

                if(var55 < 0) {
                    var55 = 0;
                }

                if(var38 > 16) {
                    var38 = 16;
                }

                bool var56 = false;

                int blockIndex;
                for(int blockX = var53; !var56 && blockX < var34; ++blockX) {
                    for(int blockZ = var55; !var56 && blockZ < var38; ++blockZ) {
                        for(int blockY = var36 + 1; !var56 && blockY >= var54 - 1; --blockY) {
                            blockIndex = (blockX * CHUNK_WIDTH_Z + blockZ) * CHUNK_HEIGHT + blockY;
                            if(blockY >= 0 && blockY < CHUNK_HEIGHT) {
                                if(c->blocks[blockIndex].type == BLOCK_WATER_FLOWING ||
                                    c->blocks[blockIndex].type == BLOCK_WATER_STILL) {
                                    var56 = true;
                                }

                                if(blockY != var54 - 1 && blockX != var53 && blockX != var34 - 1 && blockZ != var55 && blockZ != var38 - 1) {
                                    blockY = var54;
                                }
                            }
                        }
                    }
                }

                if(!var56) {
                    for(int blockX = var53; blockX < var34; ++blockX) {
                        double var57 = ((double)(blockX + cX * 16) + 0.5D - xOffset) / var27;

                        for(int blockZ = var55; blockZ < var38; ++blockZ) {
                            double var44 = ((double)(blockZ + cZ * 16) + 0.5D - zOffset) / var27;
                            int blockIndex = (blockX * CHUNK_WIDTH_Z + blockZ) * CHUNK_HEIGHT + var36;
                            bool var47 = false;
                            if(var57 * var57 + var44 * var44 < 1.0D) {
                                for(int var48 = var36 - 1; var48 >= var54; --var48) {
                                    double var49 = ((double)var48 + 0.5D - yOffset) / var29;
                                    if(var49 > -0.7D && var57 * var57 + var49 * var49 + var44 * var44 < 1.0D) {
                                        uint8_t blockType = c->blocks[blockIndex].type;
                                        if(blockType == BLOCK_GRASS) {
                                            var47 = true;
                                        }

                                        if(blockType == BLOCK_STONE ||
                                            blockType == BLOCK_DIRT ||
                                            blockType == BLOCK_GRASS) {
                                            if(var48 < 10) {
                                                c->blocks[blockIndex].type = (uint8_t)BLOCK_LAVA_FLOWING;
                                            } else {
                                                c->blocks[blockIndex].type = BLOCK_AIR;
                                                if(var47 && c->blocks[blockIndex - 1].type == BLOCK_DIRT) {
                                                    c->blocks[blockIndex - 1].type = (uint8_t)BLOCK_GRASS;
                                                }
                                            }
                                        }
                                    }

                                    --blockIndex;
                                }
                            }
                        }
                    }

                    if(var52) {
                        break;
                    }
                }
            }
        }
    }

}