#include "generatorBeta173.h"

GeneratorBeta173::GeneratorBeta173(int64_t seed, World* world) : Generator(seed, world) {
	logger = &Betrock::Logger::Instance();
    this->seed = seed;
    this->world = world;

    rand = std::make_unique<JavaRandom>(this->seed);

    // Init Terrain Noise
    lowNoiseGen         = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    highNoiseGen        = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    selectorNoiseGen    = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);
    sandGravelNoiseGen  = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
    stoneNoiseGen       = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
    noiseGen1           = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 10);
    depthNoiseGen       = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    mobSpawnerNoiseGen  = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);

    // Init Biome Noise
    std::unique_ptr<JavaRandom> rand1 = std::make_unique<JavaRandom>(this->seed * 9871L);
    temperatureNoiseGen = std::make_unique<NoiseOctaves<NoiseSimplex>>(rand1.get(), 4);
    std::unique_ptr<JavaRandom> rand2 = std::make_unique<JavaRandom>(this->seed * 39811L);
    humidityNoiseGen    = std::make_unique<NoiseOctaves<NoiseSimplex>>(rand2.get(), 4);
    std::unique_ptr<JavaRandom> rand3 = std::make_unique<JavaRandom>(this->seed * 543321L);
    weirdnessNoiseGen   = std::make_unique<NoiseOctaves<NoiseSimplex>>(rand3.get(), 2);
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
    double oneThirtySecond = 1.0D / 32.0D;
    // Init noise maps
    this->sandNoise.resize(256, 0.0);
    this->gravelNoise.resize(256, 0.0);
    this->stoneNoise.resize(256, 0.0);

    // Populate noise maps
    this->sandNoise = this->sandGravelNoiseGen->GenerateOctaves(
        this->sandNoise,
        (double)(cX * CHUNK_WIDTH_X), (double)(cZ * CHUNK_WIDTH_Z),
        0.0D, 16, 16, 1,
        oneThirtySecond, oneThirtySecond, 1.0D
    );
    this->gravelNoise = this->sandGravelNoiseGen->GenerateOctaves(
        this->gravelNoise,
        (double)(cX * CHUNK_WIDTH_X), 109.0134D, (double)(cZ * CHUNK_WIDTH_Z),
        16, 1, 16,
        oneThirtySecond, 1.0D, oneThirtySecond
    );
    this->stoneNoise = this->stoneNoiseGen->GenerateOctaves(
        this->stoneNoise,
        (double)(cX * CHUNK_WIDTH_X), (double)(cZ * CHUNK_WIDTH_Z), 0.0D,
        16, 16, 1,
        oneThirtySecond * 2.0D, oneThirtySecond * 2.0D, oneThirtySecond * 2.0D
    );

    // Iterate through entire chunk
    for(int x = 0; x < CHUNK_WIDTH_X; ++x) {
        for(int z = 0; z < CHUNK_WIDTH_Z; ++z) {
            Biome biome = biomeMap[x + z * 16];
            bool sandActive = this->sandNoise[x + z * CHUNK_WIDTH_X] + this->rand->nextDouble() * 0.2D > 0.0D;
            bool gravelActive = this->gravelNoise[x + z * CHUNK_WIDTH_X] + this->rand->nextDouble() * 0.2D > 3.0D;
            int stoneActive = (int)(this->stoneNoise[x + z * CHUNK_WIDTH_X] / 3.0D + 3.0D + this->rand->nextDouble() * 0.25D);
            int var14 = -1;
            // Get biome-appropriate top and filler blocks
            uint8_t topBlock = GetTopBlock(biome);
            uint8_t fillerBlock = GetFillerBlock(biome);

            // Iterate over column top to bottom
            for(int y = CHUNK_HEIGHT-1; y >= 0; --y) {
                int blockIndex = (z * CHUNK_WIDTH_X + x) * CHUNK_HEIGHT + y;
                // Place Bedrock at bottom with some randomness
                if(y <= 0 + this->rand->nextInt(5)) {
                    c->blocks[blockIndex].type = BLOCK_BEDROCK;
                } else {
                    uint8_t currentBlock = c->blocks[blockIndex].type;
                    // Ignore air
                    if(currentBlock == BLOCK_AIR) {
                        var14 = -1;
                    // If we counter stone, start replacing it
                    } else if(currentBlock == BLOCK_STONE) {
                        if(var14 == -1) {
                            if(stoneActive <= 0) {
                                topBlock = BLOCK_AIR;
                                fillerBlock = BLOCK_STONE;
                            } else if(y >= WATER_LEVEL - 4 && y <= WATER_LEVEL + 1) {
                                // If we're close to the water level, apply gravel and sand
                                topBlock = GetTopBlock(biome);
                                fillerBlock = GetFillerBlock(biome);

                                if(gravelActive) topBlock = BLOCK_AIR;
                                if(gravelActive) fillerBlock = BLOCK_GRAVEL;
                                if(sandActive) topBlock = BLOCK_SAND;
                                if(sandActive) fillerBlock = BLOCK_SAND;
                            }

                            // Add water if we're below water level
                            if(y < WATER_LEVEL && topBlock == 0) {
                                topBlock = (uint8_t)BLOCK_WATER_STILL;
                            }

                            var14 = stoneActive;
                            if(y >= WATER_LEVEL - 1) {
                                c->blocks[blockIndex].type = topBlock;
                            } else {
                                c->blocks[blockIndex].type = fillerBlock;
                            }
                        } else if(var14 > 0) {
                            --var14;
                            c->blocks[blockIndex].type = fillerBlock;
                            if(var14 == 0 && fillerBlock == BLOCK_SAND) {
                                var14 = this->rand->nextInt(4);
                                fillerBlock = BLOCK_SANDSTONE;
                            }
                        }
                    }
                }
            }
        }
    }

}

void GeneratorBeta173::GenerateTerrain(int cX, int cZ, std::unique_ptr<Chunk>& c, std::vector<Biome> biomeMap, std::vector<double>& temperature) {
    int xMax = 4 + 1;
    uint8_t yMax = 16 + 1;
    int zMax = 4 + 1;
    
    // Terrain noise is interpolated and only sampled every 4 blocks
    this->terrainNoiseField = this->GenerateTerrainNoise(this->terrainNoiseField, cX * 4, 0, cZ * 4, xMax, yMax, zMax);

    for(int macroX = 0; macroX < 4; ++macroX) {
        for(int macroZ = 0; macroZ < 4; ++macroZ) {
            for(int macroY = 0; macroY < 16; ++macroY) {
                double eigthScaler = 0.125D;
                double xIndex =  this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 0) * yMax + macroY + 0];
                double var18 =  this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 1) * yMax + macroY + 0];
                double var20 =  this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 0) * yMax + macroY + 0];
                double var22 =  this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 1) * yMax + macroY + 0];
                double var24 = (this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 0) * yMax + macroY + 1] - xIndex) * eigthScaler;
                double var26 = (this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 1) * yMax + macroY + 1] - var18) * eigthScaler;
                double var28 = (this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 0) * yMax + macroY + 1] - var20) * eigthScaler;
                double var30 = (this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 1) * yMax + macroY + 1] - var22) * eigthScaler;

                // Interpolate the 1/4th scale noise
                for(int subY = 0; subY < 8; ++subY) {
                    double quarterScale = 0.25D;
                    double var35 = xIndex;
                    double var37 = var18;
                    double var39 = (var20 - xIndex) * quarterScale;
                    double var41 = (var22 - var18) * quarterScale;

                    for(int subX = 0; subX < 4; ++subX) {
                        int blockIndex = subX + macroX * 4 << 11 | 0 + macroZ * 4 << 7 | macroY * 8 + subY;
                        double var48 = var35;
                        double waterLevel0 = (var37 - var35) * quarterScale;

                        for(int subZ = 0; subZ < 4; ++subZ) {
                            double temp = temperature[(macroX * 4 + subX) * 16 + macroZ * 4 + subZ];
                            int blockType = 0;
                            if(macroY * 8 + subY < WATER_LEVEL) {
                                if(temp < 0.5D && macroY * 8 + subY >= WATER_LEVEL - 1) {
                                    blockType = BLOCK_ICE;
                                } else {
                                    blockType = BLOCK_WATER_STILL;
                                }
                            }

                            if(var48 > 0.0D) {
                                blockType = BLOCK_STONE;
                            }
                            
                            c->blocks[blockIndex].type = (uint8_t)blockType;
                            blockIndex += CHUNK_HEIGHT;
                            var48 += waterLevel0;
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

    this->temperature = this->temperatureNoiseGen->GenerateOctaves(this->temperature, (double)x, (double)z, xMax, zMax, (double)0.025F, (double)0.025F, 0.25D);
    this->humidity = this->humidityNoiseGen->GenerateOctaves(this->humidity, (double)x, (double)z, xMax, zMax, (double)0.05F, (double)0.05F, 1.0D / 3.0D);
    this->weirdness = this->weirdnessNoiseGen->GenerateOctaves(this->weirdness, (double)x, (double)z, xMax, zMax, 0.25D, 0.25D, 0.5882352941176471D);
    int index = 0;

    for(int iX = 0; iX < xMax; ++iX) {
        for(int iZ = 0; iZ < zMax; ++iZ) {
            double weird = this->weirdness[index] * 1.1D + 0.5D;
            double scale = 0.01D;
            double max = 1.0D - scale;
            double temp = (this->temperature[index] * 0.15D + 0.7D) * max + weird * scale;
            scale = 0.002D;
            max = 1.0D - scale;
            double humi = (this->humidity[index] * 0.15D + 0.5D) * max + weird * scale;
            temp = 1.0D - (1.0D - temp) * (1.0D - temp);
            // Limit values to 0.0 - 1.0
            if(temp < 0.0D) temp = 0.0D;
            if(humi < 0.0D) humi = 0.0D;
            if(temp > 1.0D) temp = 1.0D;
            if(humi > 1.0D) humi = 1.0D;

            this->temperature[index] = temp;
            this->humidity[index] = humi;
            // Get the biome from the lookup
            biomeMap[index] = GetBiomeFromLookup(temp, humi);
            index++;
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
    this->noiseField1 = this->noiseGen1->GenerateOctaves(this->noiseField1, cX, cZ, xMax, zMax, 1.121D, 1.121D, 0.5D);
    this->depthNoiseField = this->depthNoiseGen->GenerateOctaves(this->depthNoiseField, cX, cZ, xMax, zMax, 200.0D, 200.0D, 0.5D);
    this->selectorNoiseField = this->selectorNoiseGen->GenerateOctaves(this->selectorNoiseField, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax, horiScale / 80.0D, vertScale / 160.0D, horiScale / 80.0D);
    this->lowNoiseField = this->lowNoiseGen->GenerateOctaves(this->lowNoiseField, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax, horiScale, vertScale, horiScale);
    this->highNoiseField = this->highNoiseGen->GenerateOctaves(this->highNoiseField, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax, horiScale, vertScale, horiScale);
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

            double var29 = this->depthNoiseField[zIndex] / 8000.0D;
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

                double var38 = this->lowNoiseField[yIndex] / 512.0D;
                double var40 = this->highNoiseField[yIndex] / 512.0D;
                double var42 = (this->selectorNoiseField[yIndex] / 10.0D + 1.0D) / 2.0D;
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