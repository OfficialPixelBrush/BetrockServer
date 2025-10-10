#include "generatorBeta173.h"

GeneratorBeta173::GeneratorBeta173(int64_t seed, World* world) : Generator(seed, world) {
	logger = &Betrock::Logger::Instance();
    this->seed = seed;
    this->world = world;

    rand = std::make_unique<JavaRandom>(this->seed);

    // Init Terrain Noise
    noiseGen1 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    noiseGen2 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    noiseGen3 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);
    noiseGen4 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
    noiseGen5 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
    noiseGen6 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 10);
    noiseGen7 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    mobSpawnerNoise = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);

    // Init Biome Noise
    std::unique_ptr<JavaRandom> rand1 = std::make_unique<JavaRandom>(this->seed * 9871L);
    noiseSimplex1 = std::make_unique<NoiseOctaves<NoiseSimplex>>(rand1.get(), 4);
    std::unique_ptr<JavaRandom> rand2 = std::make_unique<JavaRandom>(this->seed * 39811L);
    noiseSimplex2 = std::make_unique<NoiseOctaves<NoiseSimplex>>(rand2.get(), 4);
    std::unique_ptr<JavaRandom> rand3 = std::make_unique<JavaRandom>(this->seed * 543321L);
    noiseSimplex3 = std::make_unique<NoiseOctaves<NoiseSimplex>>(rand3.get(), 2);
}

std::unique_ptr<Chunk> GeneratorBeta173::GenerateChunk(int32_t cX, int32_t cZ) {
    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(this->world,cX,cZ);
    this->rand->setSeed((long)cX * 341873128712L + (long)cZ * 132897987541L);
    std::memset(c->blocks, 0, sizeof(c->blocks));

    // this->biomesForGeneration = this->worldObj.getWorldChunkManager().loadBlockGeneratorData(this->biomesForGeneration, var1 * 16, var2 * 16, 16, 16);
	this->biomeMap = GenerateBiomeMap(
        this->biomeMap,
        cX * CHUNK_WIDTH_X,
        cZ * CHUNK_WIDTH_Z,
        CHUNK_WIDTH_X,
        CHUNK_WIDTH_Z
    );

    GenerateTerrain(cX, cZ, c, this->biomeMap, this->temperature);
    ReplaceBlocksForBiome(cX, cZ, c, this->biomeMap);
    //this->field_695_u.func_667_a(this, this->worldObj, var1, var2, var3);
    //var4.func_353_b();
    
    c->GenerateHeightMap();
    c->state = ChunkState::Generated;
    c->modified = true;
    return c;
}

bool GeneratorBeta173::PopulateChunk(int32_t cX, int32_t cZ) {
    return true;
}

void GeneratorBeta173::ReplaceBlocksForBiome(int cX, int cZ, std::unique_ptr<Chunk>& c, std::vector<Biome> biomeMap) {
    uint8_t var5 = 64;
    double var6 = 1.0D / 32.0D;
    this->sandNoise.resize(256, 0.0);
    this->gravelNoise.resize(256, 0.0);
    this->stoneNoise.resize(256, 0.0);
    
    this->sandNoise = this->noiseGen4->GenerateOctaves(this->sandNoise, (double)(cX * 16), (double)(cZ * 16), 0.0D, 16, 16, 1, var6, var6, 1.0D);
    this->gravelNoise = this->noiseGen4->GenerateOctaves(this->gravelNoise, (double)(cX * 16), 109.0134D, (double)(cZ * 16), 16, 1, 16, var6, 1.0D, var6);
    this->stoneNoise = this->noiseGen5->GenerateOctaves(this->stoneNoise, (double)(cX * 16), (double)(cZ * 16), 0.0D, 16, 16, 1, var6 * 2.0D, var6 * 2.0D, var6 * 2.0D);

    for(int var8 = 0; var8 < 16; ++var8) {
        for(int var9 = 0; var9 < 16; ++var9) {
            Biome var10 = biomeMap[var8 + var9 * 16];
            bool var11 = this->sandNoise[var8 + var9 * 16] + this->rand->nextDouble() * 0.2D > 0.0D;
            bool var12 = this->gravelNoise[var8 + var9 * 16] + this->rand->nextDouble() * 0.2D > 3.0D;
            int var13 = (int)(this->stoneNoise[var8 + var9 * 16] / 3.0D + 3.0D + this->rand->nextDouble() * 0.25D);
            int var14 = -1;
            uint8_t var15 = BLOCK_GRASS; //var10.topBlock;
            uint8_t var16 = BLOCK_DIRT; //var10.fillerBlock;

            for(int var17 = 127; var17 >= 0; --var17) {
                int blockIndex = (var9 * 16 + var8) * 128 + var17;
                if(var17 <= 0 + this->rand->nextInt(5)) {
                    c->blocks[blockIndex].type = (uint8_t)BLOCK_BEDROCK;
                } else {
                    uint8_t var19 = c->blocks[blockIndex].type;
                    if(var19 == 0) {
                        var14 = -1;
                    } else if(var19 == BLOCK_STONE) {
                        if(var14 == -1) {
                            if(var13 <= 0) {
                                var15 = 0;
                                var16 = (uint8_t)BLOCK_STONE;
                            } else if(var17 >= var5 - 4 && var17 <= var5 + 1) {
                                var15 = BLOCK_GRASS; //var10.topBlock;
                                var16 = BLOCK_DIRT; //var10.fillerBlock;
                                if(var12) {
                                    var15 = 0;
                                }

                                if(var12) {
                                    var16 = (uint8_t)BLOCK_GRAVEL;
                                }

                                if(var11) {
                                    var15 = (uint8_t)BLOCK_SAND;
                                }

                                if(var11) {
                                    var16 = (uint8_t)BLOCK_SAND;
                                }
                            }

                            if(var17 < var5 && var15 == 0) {
                                var15 = (uint8_t)BLOCK_WATER_STILL;
                            }

                            var14 = var13;
                            if(var17 >= var5 - 1) {
                                c->blocks[blockIndex].type = var15;
                            } else {
                                c->blocks[blockIndex].type = var16;
                            }
                        } else if(var14 > 0) {
                            --var14;
                            c->blocks[blockIndex].type = var16;
                            if(var14 == 0 && var16 == BLOCK_SAND) {
                                var14 = this->rand->nextInt(4);
                                var16 = (uint8_t)BLOCK_SANDSTONE;
                            }
                        }
                    }
                }
            }
        }
    }

}

void GeneratorBeta173::GenerateTerrain(int cX, int cZ, std::unique_ptr<Chunk>& c, std::vector<Biome> biomeMap, std::vector<double>& temperature) {//, BiomeGenBase[] var4, double[] var5) {
    uint8_t waterLevel = 64;
    int xMax = 4 + 1;
    uint8_t yMax = 16 + 1;
    int zMax = 4 + 1;
    
    // Terrain noise is interpolated and only sampled every 4 blocks
    this->terrainNoise = this->GenerateTerrainNoise(this->terrainNoise, cX * 4, 0, cZ * 4, xMax, yMax, zMax);

    for(int macroX = 0; macroX < 4; ++macroX) {
        for(int macroZ = 0; macroZ < 4; ++macroZ) {
            for(int macroY = 0; macroY < 16; ++macroY) {
                double eigthScaler = 0.125D;
                double xIndex =  this->terrainNoise[((macroX + 0) * zMax + macroZ + 0) * yMax + macroY + 0];
                double var18 =  this->terrainNoise[((macroX + 0) * zMax + macroZ + 1) * yMax + macroY + 0];
                double var20 =  this->terrainNoise[((macroX + 1) * zMax + macroZ + 0) * yMax + macroY + 0];
                double var22 =  this->terrainNoise[((macroX + 1) * zMax + macroZ + 1) * yMax + macroY + 0];
                double var24 = (this->terrainNoise[((macroX + 0) * zMax + macroZ + 0) * yMax + macroY + 1] - xIndex) * eigthScaler;
                double var26 = (this->terrainNoise[((macroX + 0) * zMax + macroZ + 1) * yMax + macroY + 1] - var18) * eigthScaler;
                double var28 = (this->terrainNoise[((macroX + 1) * zMax + macroZ + 0) * yMax + macroY + 1] - var20) * eigthScaler;
                double var30 = (this->terrainNoise[((macroX + 1) * zMax + macroZ + 1) * yMax + macroY + 1] - var22) * eigthScaler;

                // Interpolate the 1/4th scale noise
                for(int subY = 0; subY < 8; ++subY) {
                    double quarterScale = 0.25D;
                    double var35 = xIndex;
                    double var37 = var18;
                    double var39 = (var20 - xIndex) * quarterScale;
                    double var41 = (var22 - var18) * quarterScale;

                    for(int subX = 0; subX < 4; ++subX) {
                        int blockIndex = subX + macroX * 4 << 11 | 0 + macroZ * 4 << 7 | macroY * 8 + subY;
                        short worldHeight = 128;
                        double var46 = 0.25D;
                        double var48 = var35;
                        double var50 = (var37 - var35) * var46;

                        for(int subZ = 0; subZ < 4; ++subZ) {
                            double var53 = temperature[(macroX * 4 + subX) * 16 + macroZ * 4 + subZ];
                            int blockType = 0;
                            if(macroY * 8 + subY < waterLevel) {
                                if(var53 < 0.5D && macroY * 8 + subY >= waterLevel - 1) {
                                    blockType = BLOCK_ICE;
                                } else {
                                    blockType = BLOCK_WATER_STILL;
                                }
                            }

                            if(var48 > 0.0D) {
                                blockType = BLOCK_STONE;
                            }
                            
                            c->blocks[blockIndex].type = (uint8_t)blockType;
                            blockIndex += worldHeight;
                            var48 += var50;
                        }

                        var35 += var39;
                        var37 += var41;
                    }

                    xIndex += var24;
                    var18 += var26;
                    var20 += var28;
                    var22 += var30;
                }
            }
        }
    }
}

std::vector<Biome> GeneratorBeta173::GenerateBiomeMap(std::vector<Biome> biomeMap, int x, int z, int xMax, int zMax) {
    if(biomeMap.empty() || biomeMap.size() < xMax * zMax) {
        biomeMap.resize(xMax * zMax, BIOME_NONE);
    }

    this->temperature = this->noiseSimplex1->GenerateOctaves(this->temperature, (double)x, (double)z, xMax, zMax, (double)0.025F, (double)0.025F, 0.25D);
    this->humidity = this->noiseSimplex2->GenerateOctaves(this->humidity, (double)x, (double)z, xMax, zMax, (double)0.05F, (double)0.05F, 1.0D / 3.0D);
    this->otherBiomeThing = this->noiseSimplex3->GenerateOctaves(this->otherBiomeThing, (double)x, (double)z, xMax, zMax, 0.25D, 0.25D, 0.5882352941176471D);
    int index = 0;

    for(int iX = 0; iX < xMax; ++iX) {
        for(int iZ = 0; iZ < zMax; ++iZ) {
            double var9 = this->otherBiomeThing[index] * 1.1D + 0.5D;
            double var11 = 0.01D;
            double var13 = 1.0D - var11;
            double temp = (this->temperature[index] * 0.15D + 0.7D) * var13 + var9 * var11;
            var11 = 0.002D;
            var13 = 1.0D - var11;
            double humi = (this->humidity[index] * 0.15D + 0.5D) * var13 + var9 * var11;
            temp = 1.0D - (1.0D - temp) * (1.0D - temp);
            // Limit values to 0.0 - 1.0
            if(temp < 0.0D) temp = 0.0D;
            if(humi < 0.0D) humi = 0.0D;
            if(temp > 1.0D) temp = 1.0D;
            if(humi > 1.0D) humi = 1.0D;

            this->temperature[index] = temp;
            this->humidity[index] = humi;
            biomeMap[index++] = GetBiomeFromLookup(temp, humi); //;BiomeGenBase.getBiomeFromLookup(temp, humi);
        }
    }

    return biomeMap;
}

std::vector<double> GeneratorBeta173::GenerateTerrainNoise(std::vector<double> terrainMap, int cX, int cY, int cZ, int xMax, int yMax, int zMax) {
    if(terrainMap.empty()) {
        terrainMap.resize(xMax * yMax * zMax, 0.0);
    }

    double horiScale = 684.412D;
    double vertScale = 684.412D;
    std::vector<double> var12 = this->temperature;
    std::vector<double> var13 = this->humidity;
    
    // We do this to need to generate noise as often
    this->noiseField1 = this->noiseGen6->GenerateOctaves(this->noiseField1, cX, cZ, xMax, zMax, 1.121D, 1.121D, 0.5D);
    this->noiseField2 = this->noiseGen7->GenerateOctaves(this->noiseField2, cX, cZ, xMax, zMax, 200.0D, 200.0D, 0.5D);
    this->noiseField3 = this->noiseGen3->GenerateOctaves(this->noiseField3, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax, horiScale / 80.0D, vertScale / 160.0D, horiScale / 80.0D);
    this->noiseField4 = this->noiseGen1->GenerateOctaves(this->noiseField4, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax, horiScale, vertScale, horiScale);
    this->noiseField5 = this->noiseGen2->GenerateOctaves(this->noiseField5, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax, horiScale, vertScale, horiScale);
    int yIndex = 0;
    int zIndex = 0;
    int xIndex = 16 / xMax;

    for(int iX = 0; iX < xMax; ++iX) {
        int var18 = iX * xIndex + xIndex / 2;

        for(int iZ = 0; iZ < zMax; ++iZ) {
            int var20 = iZ * xIndex + xIndex / 2;
            double var21 = var12[var18 * 16 + var20];
            double var23 = var13[var18 * 16 + var20] * var21;
            double var25 = 1.0D - var23;
            var25 *= var25;
            var25 *= var25;
            var25 = 1.0D - var25;
            double var27 = (this->noiseField1[zIndex] + 256.0D) / 512.0D;
            var27 *= var25;
            if(var27 > 1.0D) {
                var27 = 1.0D;
            }

            double var29 = this->noiseField2[zIndex] / 8000.0D;
            if(var29 < 0.0D) {
                var29 = -var29 * 0.3D;
            }

            var29 = var29 * 3.0D - 2.0D;
            if(var29 < 0.0D) {
                var29 /= 2.0D;
                if(var29 < -1.0D) {
                    var29 = -1.0D;
                }

                var29 /= 1.4D;
                var29 /= 2.0D;
                var27 = 0.0D;
            } else {
                if(var29 > 1.0D) {
                    var29 = 1.0D;
                }

                var29 /= 8.0D;
            }

            if(var27 < 0.0D) {
                var27 = 0.0D;
            }

            var27 += 0.5D;
            var29 = var29 * (double)yMax / 16.0D;
            double var31 = (double)yMax / 2.0D + var29 * 4.0D;
            ++zIndex;

            for(int iY = 0; iY < yMax; ++iY) {
                double var34 = 0.0D;
                double var36 = ((double)iY - var31) * 12.0D / var27;
                if(var36 < 0.0D) {
                    var36 *= 4.0D;
                }

                double var38 = this->noiseField4[yIndex] / 512.0D;
                double var40 = this->noiseField5[yIndex] / 512.0D;
                double var42 = (this->noiseField3[yIndex] / 10.0D + 1.0D) / 2.0D;
                if(var42 < 0.0D) {
                    var34 = var38;
                } else if(var42 > 1.0D) {
                    var34 = var40;
                } else {
                    var34 = var38 + (var40 - var38) * var42;
                }

                var34 -= var36;
                if(iY > yMax - 4) {
                    double var44 = (double)((float)(iY - (yMax - 4)) / 3.0F);
                    var34 = var34 * (1.0D - var44) + -10.0D * var44;
                }

                terrainMap[yIndex] = var34;
                ++yIndex;
            }
        }
    }

    return terrainMap;
}