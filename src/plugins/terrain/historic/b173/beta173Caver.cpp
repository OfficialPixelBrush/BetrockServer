#include "beta173Caver.h"
Beta173Caver::Beta173Caver() {
    rand = std::make_unique<JavaRandom>();
}

void Beta173Caver::GenerateCavesForChunk(World* world, int cX, int cZ, std::unique_ptr<Chunk>& c) {
    int carveExtent = this->carveExtentLimit;
    this->rand->setSeed(world->seed);
    long xOffset = this->rand->nextLong() / 2L * 2L + 1L;
    long zOffset = this->rand->nextLong() / 2L * 2L + 1L;

    // Iterate beyond the current chunk by 8 chunks in every direction
    for (int cXoffset = cX - carveExtent; cXoffset <= cX + carveExtent; ++cXoffset) {
        for (int cZoffset = cZ - carveExtent; cZoffset <= cZ + carveExtent; ++cZoffset) {
            this->rand->setSeed(((long(cXoffset) * xOffset) + (long(cZoffset) * zOffset)) ^ world->seed);
            this->GenerateCaves(cXoffset, cZoffset, cX, cZ, c);
        }
    }
}

// TODO: This is only the cave generator for the overworld.
// The one for the nether is different!
void Beta173Caver::GenerateCaves(int cXoffset, int cZoffset, int cX, int cZ, std::unique_ptr<Chunk>& c) {
    int numberOfCaves = this->rand->nextInt(this->rand->nextInt(this->rand->nextInt(40) + 1) + 1);
    if(this->rand->nextInt(15) != 0) {
        numberOfCaves = 0;
    }

    for(int caveIndex = 0; caveIndex < numberOfCaves; ++caveIndex) {
        double xOffset = double(cXoffset * CHUNK_WIDTH_X + this->rand->nextInt(CHUNK_WIDTH_X));
        double yOffset = double(this->rand->nextInt(this->rand->nextInt(120) + 8));
        double zOffset = double(cZoffset * CHUNK_WIDTH_Z + this->rand->nextInt(CHUNK_WIDTH_Z));
        int numberOfNodes = 1;
        if(this->rand->nextInt(4) == 0) {
            this->CarveCave(cX, cZ, c, xOffset, yOffset, zOffset);
            numberOfNodes += this->rand->nextInt(4);
        }

        for(int nodeIndex = 0; nodeIndex < numberOfNodes; ++nodeIndex) {
            float carveYaw = this->rand->nextFloat() * (float)M_PI * 2.0F;
            float carvePitch = (this->rand->nextFloat() - 0.5F) * 2.0F / 8.0F;
            float tunnelRadius = this->rand->nextFloat() * 2.0F + this->rand->nextFloat();
            this->CarveCave(
                cX, cZ, c,
                xOffset, yOffset, zOffset,
                tunnelRadius, carveYaw, carvePitch,
                0, 0, 1.0D
            );
        }
    }
}

void Beta173Caver::CarveCave(int cX, int cZ, std::unique_ptr<Chunk>& c, double xOffset, double yOffset, double zOffset) {
    this->CarveCave(
        cX, cZ, c,
        xOffset, yOffset, zOffset,
        1.0F + this->rand->nextFloat() * 6.0F, 0.0F, 0.0F,
        -1, -1, 0.5D
    );
}

void Beta173Caver::CarveCave(
    int cX, int cZ, std::unique_ptr<Chunk>& c,
    double xOffset, double yOffset, double zOffset,
    float tunnelRadius, float carveYaw, float carvePitch,
    int tunnelStep, int tunnelLength, double verticalScale
) {
    double chunkCenterX = double(cX * CHUNK_WIDTH_X + 8);
    double chunkCenterZ = double(cZ * CHUNK_WIDTH_Z + 8);
    float var21 = 0.0F;
    float var22 = 0.0F;
    std::unique_ptr<JavaRandom> rand2 = std::make_unique<JavaRandom>(this->rand->nextLong());
    if(tunnelLength <= 0) {
        int var24 = this->carveExtentLimit * 16 - 16;
        tunnelLength = var24 - rand2->nextInt(var24 / 4);
    }

    bool var52 = false;
    if(tunnelStep == -1) {
        tunnelStep = tunnelLength / 2;
        var52 = true;
    }

    int var25 = rand2->nextInt(tunnelLength / 2) + tunnelLength / 4;

    for(bool var26 = rand2->nextInt(6) == 0; tunnelStep < tunnelLength; ++tunnelStep) {
        double var27 = 1.5D + (double)(MathHelper::sin((float)tunnelStep * (float)M_PI / (float)tunnelLength) * tunnelRadius * 1.0F);
        double var29 = var27 * verticalScale;
        float var31 = MathHelper::cos(carvePitch);
        float var32 = MathHelper::sin(carvePitch);
        xOffset += (double)(MathHelper::cos(carveYaw) * var31);
        yOffset += (double)var32;
        zOffset += (double)(MathHelper::sin(carveYaw) * var31);
        if(var26) {
            carvePitch *= 0.92F;
        } else {
            carvePitch *= 0.7F;
        }

        carvePitch += var22 * 0.1F;
        carveYaw += var21 * 0.1F;
        var22 *= 0.9F;
        var21 *= 12.0F / 16.0F;
        var22 += (rand2->nextFloat() - rand2->nextFloat()) * rand2->nextFloat() * 2.0F;
        var21 += (rand2->nextFloat() - rand2->nextFloat()) * rand2->nextFloat() * 4.0F;
        if(!var52 && tunnelStep == var25 && tunnelRadius > 1.0F) {
            this->CarveCave(cX, cZ, c, xOffset, yOffset, zOffset, rand2->nextFloat() * 0.5F + 0.5F, carveYaw - (float)M_PI * 0.5F, carvePitch / 3.0F, tunnelStep, tunnelLength, 1.0D);
            this->CarveCave(cX, cZ, c, xOffset, yOffset, zOffset, rand2->nextFloat() * 0.5F + 0.5F, carveYaw + (float)M_PI * 0.5F, carvePitch / 3.0F, tunnelStep, tunnelLength, 1.0D);
            return;
        }

        if(var52 || rand2->nextInt(4) != 0) {
            double var33 = xOffset - chunkCenterX;
            double var35 = zOffset - chunkCenterZ;
            double var37 = (double)(tunnelLength - tunnelStep);
            double var39 = (double)(tunnelRadius + 2.0F + 16.0F);
            if(var33 * var33 + var35 * var35 - var37 * var37 > var39 * var39) {
                return;
            }

            if(xOffset >= chunkCenterX - 16.0D - var27 * 2.0D && zOffset >= chunkCenterZ - 16.0D - var27 * 2.0D && xOffset <= chunkCenterX + 16.0D + var27 * 2.0D && zOffset <= chunkCenterZ + 16.0D + var27 * 2.0D) {
                int xMin = MathHelper::floor_double(xOffset - var27) - cX * 16 - 1;
                int xMax = MathHelper::floor_double(xOffset + var27) - cX * 16 + 1;
                int yMin = MathHelper::floor_double(yOffset - var29) - 1;
                int yMax = MathHelper::floor_double(yOffset + var29) + 1;
                int zMin = MathHelper::floor_double(zOffset - var27) - cZ * 16 - 1;
                int zMax = MathHelper::floor_double(zOffset + var27) - cZ * 16 + 1;
                // Limiting to chunk boundaries
                if(xMin < 0) xMin = 0;
                if(xMax > 16) xMax = 16;
                if(yMin < 1) yMin = 1;
                if(yMax > 120)yMax = 120;
                if(zMin < 0) zMin = 0;
                if(zMax > 16) zMax = 16;

                bool waterIsPresent = false;

                int blockIndex;
                for(int blockX = xMin; !waterIsPresent && blockX < xMax; ++blockX) {
                    for(int blockZ = zMin; !waterIsPresent && blockZ < zMax; ++blockZ) {
                        for(int blockY = yMax + 1; !waterIsPresent && blockY >= yMin - 1; --blockY) {
                            blockIndex = (blockX * CHUNK_WIDTH_Z + blockZ) * CHUNK_HEIGHT + blockY;
                            if(blockY >= 0 && blockY < CHUNK_HEIGHT) {
                                if(c->blocks[blockIndex].type == BLOCK_WATER_FLOWING ||
                                    c->blocks[blockIndex].type == BLOCK_WATER_STILL) {
                                    waterIsPresent = true;
                                }

                                if(blockY != yMin - 1 && blockX != xMin && blockX != xMax - 1 && blockZ != zMin && blockZ != zMax - 1) {
                                    blockY = yMin;
                                }
                            }
                        }
                    }
                }

                if(!waterIsPresent) {
                    for(int blockX = xMin; blockX < xMax; ++blockX) {
                        double var57 = ((double)(blockX + cX * 16) + 0.5D - xOffset) / var27;

                        for(int blockZ = zMin; blockZ < zMax; ++blockZ) {
                            double var44 = ((double)(blockZ + cZ * 16) + 0.5D - zOffset) / var27;
                            int blockIndex = (blockX * CHUNK_WIDTH_Z + blockZ) * CHUNK_HEIGHT + yMax;
                            bool var47 = false;
                            if(var57 * var57 + var44 * var44 < 1.0D) {
                                for(int var48 = yMax - 1; var48 >= yMin; --var48) {
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