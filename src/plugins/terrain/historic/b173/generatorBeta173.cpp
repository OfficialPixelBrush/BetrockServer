#include "generatorBeta173.h"

GeneratorBeta173::GeneratorBeta173(int64_t seed, World* world) : Generator(seed, world) {
	logger = &Betrock::Logger::Instance();
    this->seed = seed;
    this->world = world;

    rand = std::make_unique<JavaRandom>(this->seed);

    // Init Terrain Noise
    lowNoiseGen             = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    highNoiseGen            = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    selectorNoiseGen        = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);
    sandGravelNoiseGen      = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
    stoneNoiseGen           = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
    continentalnessNoiseGen = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 10);
    depthNoiseGen           = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    treeDensityNoiseGen      = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);

    // Init Biome Noise
    auto randTemp = std::make_unique<JavaRandom>(this->seed * 9871L);
    auto randHum  = std::make_unique<JavaRandom>(this->seed * 39811L);
    auto randWeird = std::make_unique<JavaRandom>(this->seed * 543321L);
    temperatureNoiseGen = std::make_unique<NoiseOctaves<NoiseSimplex>>(randTemp.get(), 4);
    humidityNoiseGen    = std::make_unique<NoiseOctaves<NoiseSimplex>>(randHum.get(), 4);
    weirdnessNoiseGen   = std::make_unique<NoiseOctaves<NoiseSimplex>>(randWeird.get(), 2);

    // Init Caver
    caver = std::make_unique<Beta173Caver>();
}

std::unique_ptr<Chunk> GeneratorBeta173::GenerateChunk(int32_t cX, int32_t cZ) {
    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(this->world,cX,cZ);
    this->rand->setSeed((long)cX * 341873128712L + (long)cZ * 132897987541L);
    
    // Allocate empty chunk
    std::fill(std::begin(c->blocks), std::end(c->blocks), Block{BLOCK_AIR});

    // Generate Biomes
	this->biomeMap = GenerateBiomeMap(
        cX * CHUNK_WIDTH_X,
        cZ * CHUNK_WIDTH_Z,
        CHUNK_WIDTH_X,
        CHUNK_WIDTH_Z
    );
    
    /*
    if (cX >= -1 && cZ >= -1 && cX <= 1 && cZ <= 1) {
        std::cout << "# " << cX << ", " << cZ << std::endl;
        std::cout << "[";
        for (size_t i = 0; i < this->temperature.size(); i++) {
            std::cout << this->temperature[i];
            if (i < this->temperature.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]," << std::endl;
    }
    */

    // Generate the Terrain, minus any caves, as just stone 
    GenerateTerrain(cX, cZ, c);
    // Replace some of the stone with Biome-appropriate blocks
    ReplaceBlocksForBiome(cX, cZ, c);
    // Carve caves
    this->caver->GenerateCavesForChunk(this->world, cX, cZ, c);    
    // Generate heightmap
    c->GenerateHeightMap();
    c->state = ChunkState::Generated;
    c->modified = true;
    return c;
}

// Replace some of the stone with Biome-appropriate blocks
void GeneratorBeta173::ReplaceBlocksForBiome(int cX, int cZ, std::unique_ptr<Chunk>& c) {
    const double oneThirtySecond = 1.0D / 32.0D;
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
            // Get values from noise maps
            Biome biome = biomeMap[x + z * 16];
            bool sandActive = this->sandNoise[x + z * CHUNK_WIDTH_X] + this->rand->nextDouble() * 0.2D > 0.0D;
            bool gravelActive = this->gravelNoise[x + z * CHUNK_WIDTH_X] + this->rand->nextDouble() * 0.2D > 3.0D;
            int stoneActive = (int)(this->stoneNoise[x + z * CHUNK_WIDTH_X] / 3.0D + 3.0D + this->rand->nextDouble() * 0.25D);
            int stoneDepth = -1;
            // Get biome-appropriate top and filler blocks
            uint8_t topBlock = GetTopBlock(biome);
            uint8_t fillerBlock = GetFillerBlock(biome);

            // Iterate over column top to bottom
            for(int y = CHUNK_HEIGHT-1; y >= 0; --y) {
                int blockIndex = (z * CHUNK_WIDTH_X + x) * CHUNK_HEIGHT + y;
                // Place Bedrock at bottom with some randomness
                if(y <= 0 + this->rand->nextInt(5)) {
                    c->blocks[blockIndex].type = BLOCK_BEDROCK;
                    continue;
                }

                uint8_t currentBlock = c->blocks[blockIndex].type;
                // Ignore air
                if(currentBlock == BLOCK_AIR) {
                    stoneDepth = -1;
                    continue;
                }
                
                // If we counter stone, start replacing it
                if(currentBlock == BLOCK_STONE) {
                    if(stoneDepth == -1) {
                        if(stoneActive <= 0) {
                            topBlock = BLOCK_AIR;
                            fillerBlock = BLOCK_STONE;
                        } else if(y >= WATER_LEVEL - 4 && y <= WATER_LEVEL + 1) {
                            // If we're close to the water level, apply gravel and sand
                            topBlock = GetTopBlock(biome);
                            fillerBlock = GetFillerBlock(biome);

                            if(gravelActive) topBlock    = BLOCK_AIR;
                            if(gravelActive) fillerBlock = BLOCK_GRAVEL;
                            if(sandActive)   topBlock    = BLOCK_SAND;
                            if(sandActive)   fillerBlock = BLOCK_SAND;
                        }

                        // Add water if we're below water level
                        if(y < WATER_LEVEL && topBlock == BLOCK_AIR) {
                            topBlock = (uint8_t)BLOCK_WATER_STILL;
                        }

                        stoneDepth = stoneActive;
                        if(y >= WATER_LEVEL - 1) {
                            c->blocks[blockIndex].type = topBlock;
                        } else {
                            c->blocks[blockIndex].type = fillerBlock;
                        }
                    } else if(stoneDepth > 0) {
                        --stoneDepth;
                        c->blocks[blockIndex].type = fillerBlock;
                        if(stoneDepth == 0 && fillerBlock == BLOCK_SAND) {
                            stoneDepth = this->rand->nextInt(4);
                            fillerBlock = BLOCK_SANDSTONE;
                        }
                    }
                }
            }
        }
    }
}

// Generate the Terrain, minus any caves, as just stone 
void GeneratorBeta173::GenerateTerrain(int cX, int cZ, std::unique_ptr<Chunk>& c) {
    const int     xMax = CHUNK_WIDTH_X / 4 + 1; // 3
    const uint8_t yMax = CHUNK_HEIGHT  / 8 + 1; // 14
    const int     zMax = CHUNK_WIDTH_Z / 4 + 1; // 3
    
    // Generate 4x16x4 low resolution noise map
    this->terrainNoiseField = this->GenerateTerrainNoise(
        this->terrainNoiseField,
        cX * 4,
        0,
        cZ * 4,
        xMax,
        yMax,
        zMax
    );

    // Terrain noise is interpolated and only sampled every 4 blocks
    for(int macroX = 0; macroX < 4; ++macroX) {
        for(int macroZ = 0; macroZ < 4; ++macroZ) {
            for(int macroY = 0; macroY < 16; ++macroY) {
                double verticalLerpStep = 0.125D;

                // Get noise cube corners
                double corner000 =  this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 0) * yMax + macroY + 0];
                double corner010 =  this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 1) * yMax + macroY + 0];
                double corner100 =  this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 0) * yMax + macroY + 0];
                double corner110 =  this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 1) * yMax + macroY + 0];
                double corner001 = (this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 0) * yMax + macroY + 1] - corner000) * verticalLerpStep;
                double corner011 = (this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 1) * yMax + macroY + 1] - corner010) * verticalLerpStep;
                double corner101 = (this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 0) * yMax + macroY + 1] - corner100) * verticalLerpStep;
                double corner111 = (this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 1) * yMax + macroY + 1] - corner110) * verticalLerpStep;

                // Interpolate the 1/4th scale noise
                for(int subY = 0; subY < 8; ++subY) {
                    double horizontalLerpStep = 0.25D;
                    double terrainX0 = corner000;
                    double terrainX1 = corner010;
                    double terrainStepX0 = (corner100 - corner000) * horizontalLerpStep;
                    double terrainStepX1 = (corner110 - corner010) * horizontalLerpStep;

                    for(int subX = 0; subX < 4; ++subX) {
                        int blockIndex = ((subX + macroX * 4) << 11) 
                                | ((macroZ * 4) << 7) 
                                | ((macroY * 8) + subY);
                        double terrainDensity = terrainX0;
                        double densityStepZ = (terrainX1 - terrainX0) * horizontalLerpStep;

                        for(int subZ = 0; subZ < 4; ++subZ) {
                            // Here the actual block is determined
                            // Default to air block
                            uint8_t blockType = BLOCK_AIR;

                            // If water is too cold, turn into ice
                            double temp = this->temperature[(macroX * 4 + subX) * 16 + macroZ * 4 + subZ];
                            int yLevel = macroY * 8 + subY;
                            if(yLevel < WATER_LEVEL) {
                                if(temp < 0.5D && yLevel >= WATER_LEVEL - 1) {
                                    blockType = BLOCK_ICE;
                                } else {
                                    blockType = BLOCK_WATER_STILL;
                                }
                            }

                            // If the terrain density falls below,
                            // replace block with stone
                            if(terrainDensity > 0.0D) {
                                blockType = BLOCK_STONE;
                            }
                            
                            c->blocks[blockIndex].type = blockType;
                            // Prep for next iteration
                            blockIndex += CHUNK_HEIGHT;
                            terrainDensity += densityStepZ;
                        }
                        

                        terrainX0 += terrainStepX0;
                        terrainX1 += terrainStepX1;
                    }

                    corner000 += corner001;
                    corner010 += corner011;
                    corner100 += corner101;
                    corner110 += corner111;
                }
            }
        }
    }
}

std::vector<double> GeneratorBeta173::GenerateTemperature(int bx, int bz, int xMax, int zMax) {
    if(this->temperature.empty() || this->temperature.size() < size_t(xMax * zMax)) {
        this->temperature.resize(xMax * zMax, 0.0);
    }

    this->temperature = this->temperatureNoiseGen->GenerateOctaves(this->temperature, (double)bx, (double)bz, xMax, zMax, (double)0.025F, (double)0.025F, 0.25D);
    this->weirdness = this->weirdnessNoiseGen->GenerateOctaves(this->weirdness, (double)bx, (double)bz, xMax, zMax, 0.25D, 0.25D, 0.5882352941176471D);
    int index = 0;

    for(int x = 0; x < xMax; ++x) {
        for(int z = 0; z < zMax; ++z) {
            double var9 = this->weirdness[index] * 1.1D + 0.5D;
            double scale = 0.01D;
            double max = 1.0D - scale;
            double temp = (this->temperature[index] * 0.15D + 0.7D) * max + var9 * scale;
            temp = 1.0D - (1.0D - temp) * (1.0D - temp);
            if(temp < 0.0D) {
                temp = 0.0D;
            }

            if(temp > 1.0D) {
                temp = 1.0D;
            }

            this->temperature[index] = temp;
            ++index;
        }
    }

    return this->temperature;
}

// Generate Biomes based on simplex noise
std::vector<Biome> GeneratorBeta173::GenerateBiomeMap(int bx, int bz, int xMax, int zMax) {
    // Init Biome map
    if(this->biomeMap.empty() || int(this->biomeMap.size()) < xMax * zMax) {
        this->biomeMap.resize(xMax * zMax, BIOME_NONE);
    }

    // Get noise values
    // We found an oversight in the original code! zMax is NEVER used for getting the noise range!
    // Although this is irrelevant for all intents and purposes, as xMax always equals zMax
    this->temperature = this->temperatureNoiseGen->GenerateOctaves(this->temperature, (double)bx, (double)bz, xMax, xMax, 0.025D, 0.025D, 0.25D);
    this->humidity = this->humidityNoiseGen->GenerateOctaves(this->humidity, (double)bx, (double)bz, xMax, xMax, 0.05D, 0.05D, 1.0D / 3.0D);
    this->weirdness = this->weirdnessNoiseGen->GenerateOctaves(this->weirdness, (double)bx, (double)bz, xMax, xMax, 0.25D, 0.25D, 0.5882352941176471D);
    int index = 0;

    // Iterate over each block column
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

            // Write the temperature and humidity values back
            this->temperature[index] = temp;
            this->humidity[index] = humi;
            // Get the biome from the lookup
            this->biomeMap[index] = GetBiomeFromLookup(temp, humi);
            index++;
        }
    }
    return this->biomeMap;
}

// Make terrain noise
std::vector<double> GeneratorBeta173::GenerateTerrainNoise(std::vector<double> terrainMap, int cX, int cY, int cZ, int xMax, int yMax, int zMax) {
    if(terrainMap.empty()) {
        terrainMap.resize(xMax * yMax * zMax, 0.0);
    }

    double horiScale = 684.412D;
    double vertScale = 684.412D;
    
    // We do this to need to generate noise as often
    this->continentalnessNoiseField = this->continentalnessNoiseGen->GenerateOctaves(this->continentalnessNoiseField, cX, cZ, xMax, zMax, 1.121D, 1.121D, 0.5D);
    this->depthNoiseField = this->depthNoiseGen->GenerateOctaves(this->depthNoiseField, cX, cZ, xMax, zMax, 200.0D, 200.0D, 0.5D);
    this->selectorNoiseField = this->selectorNoiseGen->GenerateOctaves(this->selectorNoiseField, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax, horiScale / 80.0D, vertScale / 160.0D, horiScale / 80.0D);
    this->lowNoiseField = this->lowNoiseGen->GenerateOctaves(this->lowNoiseField, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax, horiScale, vertScale, horiScale);
    this->highNoiseField = this->highNoiseGen->GenerateOctaves(this->highNoiseField, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax, horiScale, vertScale, horiScale);
    // Used to iterate 3D noise maps (low, high, selector)
    int xyzIndex = 0;
    // Used to iterate 2D Noise maps (depth, continentalness)
    int xzIndex = 0;
    int scaleFraction = 16 / xMax;

    for(int iX = 0; iX < xMax; ++iX) {
        int sampleX = iX * scaleFraction + scaleFraction / 2;

        for(int iZ = 0; iZ < zMax; ++iZ) {
            // Sample 2D noises
            int sampleZ = iZ * scaleFraction + scaleFraction / 2;
            // Apply biome-noise-dependent variety
            double temp = this->temperature[sampleX * CHUNK_WIDTH_X + sampleZ];
            double humi = this->humidity[sampleX * CHUNK_WIDTH_X + sampleZ] * temp;
            humi = 1.0D - humi;
            humi *= humi;
            humi *= humi;
            humi = 1.0D - humi;
            // Apply contientalness
            double continentalness = (this->continentalnessNoiseField[xzIndex] + 256.0D) / 512.0D;
            continentalness *= humi;
            if(continentalness > 1.0D) continentalness = 1.0D;

            double depthNoise = this->depthNoiseField[xzIndex] / 8000.0D;
            if(depthNoise < 0.0D) depthNoise = -depthNoise * 0.3D;

            depthNoise = depthNoise * 3.0D - 2.0D;
            if(depthNoise < 0.0D) {
                depthNoise /= 2.0D;
                if(depthNoise < -1.0D) depthNoise = -1.0D;

                depthNoise /= 1.4D;
                depthNoise /= 2.0D;
                continentalness = 0.0D;
            } else {
                if(depthNoise > 1.0D) depthNoise = 1.0D;
                depthNoise /= 8.0D;
            }

            if(continentalness < 0.0D) {
                continentalness = 0.0D;
            }

            continentalness += 0.5D;
            depthNoise = depthNoise * (double)yMax / 16.0D;
            double elevationOffset = (double)yMax / 2.0D + depthNoise * 4.0D;
            ++xzIndex;

            for(int iY = 0; iY < yMax; ++iY) {
                // Sample 3D noises
                double terrainDensity = 0.0D;
                double densityOffset = ((double)iY - elevationOffset) * 12.0D / continentalness;
                if(densityOffset < 0.0D) {
                    densityOffset *= 4.0D;
                }

                double lowNoise = this->lowNoiseField[xyzIndex] / 512.0D;
                double highNoise = this->highNoiseField[xyzIndex] / 512.0D;
                double selectorNoise = (this->selectorNoiseField[xyzIndex] / 10.0D + 1.0D) / 2.0D;
                if(selectorNoise < 0.0D) {
                    terrainDensity = lowNoise;
                } else if(selectorNoise > 1.0D) {
                    terrainDensity = highNoise;
                } else {
                    terrainDensity = lowNoise + (highNoise - lowNoise) * selectorNoise;
                }

                terrainDensity -= densityOffset;
                // Reduce density towards max height
                if(iY > yMax - 4) {
                    double heightEdgeFade = (double)((float)(iY - (yMax - 4)) / 3.0F);
                    terrainDensity = terrainDensity * (1.0D - heightEdgeFade) + -10.0D * heightEdgeFade;
                }

                terrainMap[xyzIndex] = terrainDensity;
                ++xyzIndex;
            }
        }
    }

    return terrainMap;
}

Biome GeneratorBeta173::GetBiomeAt(int worldX, int worldZ) {
    int localX = worldX % CHUNK_WIDTH_X;
    int localZ = worldZ % CHUNK_WIDTH_Z;
    if(localX < 0) localX += CHUNK_WIDTH_X;
    if(localZ < 0) localZ += CHUNK_WIDTH_Z;
    return biomeMap[localX + localZ * CHUNK_WIDTH_X];
}


bool GeneratorBeta173::PopulateChunk(int32_t cX, int32_t cZ) {
    //BlockSand.fallInstantly = true;
    int blockX = cX * CHUNK_WIDTH_X;
    int blockZ = cZ * CHUNK_WIDTH_Z;
    Biome biome = GetBiomeAt(blockX + 16, blockZ + 16);
    this->rand->setSeed(this->world->seed);
    long xOffset = this->rand->nextLong() / 2L * 2L + 1L;
    long zOffset = this->rand->nextLong() / 2L * 2L + 1L;
    this->rand->setSeed(((long(cX) * xOffset) + (long(cZ) * zOffset)) ^ this->world->seed);
    double fraction = 0.25D;
    [[maybe_unused]] int xCoordinate;
    [[maybe_unused]] int yCoordinate;
    [[maybe_unused]] int zCoordinate;

    // Generate lakes
    if(this->rand->nextInt(4) == 0) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature(BLOCK_WATER_STILL).GenerateLake(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    // Generate lava lakes
    if(this->rand->nextInt(8) == 0) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(this->rand->nextInt(120) + 8);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        if(yCoordinate < WATER_LEVEL || this->rand->nextInt(10) == 0) {
            Beta173Feature(BLOCK_LAVA_STILL).GenerateLake(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
        }
    }

    for(int i = 0; i < 8; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature().GenerateDungeon(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    for(int i = 0; i < 10; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
        Beta173Feature().GenerateClay(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 32);
    }

    for(int i = 0; i < 20; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
        Beta173Feature(BLOCK_DIRT).GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 32);
    }

    for(int i = 0; i < 10; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
        Beta173Feature(BLOCK_GRAVEL).GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 32);
    }

    for(int i = 0; i < 20; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
        Beta173Feature(BLOCK_ORE_COAL).GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 16);
    }

    for(int i = 0; i < 20; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT/2);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
        Beta173Feature(BLOCK_ORE_IRON).GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 8);
    }

    for(int i = 0; i < 2; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT/4);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
        Beta173Feature(BLOCK_ORE_GOLD).GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 8);
    }

    for(int i = 0; i < 8; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT/8);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
        Beta173Feature(BLOCK_ORE_REDSTONE_OFF).GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 7);
    }

    for(int i = 0; i < 1; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT/8);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
        Beta173Feature(BLOCK_ORE_DIAMOND).GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 7);
    }

    for(int i = 0; i < 1; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT/8) + this->rand->nextInt(16);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
        Beta173Feature(BLOCK_ORE_LAPIS_LAZULI).GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 6);
    }

    fraction = 0.5D;
    int treeDensitySample = int(
        (
            this->treeDensityNoiseGen->GenerateOctaves(
                double(blockX) * fraction,
                double(blockZ) * fraction
            ) / 8.0D + this->rand->nextDouble() * 4.0D + 4.0D
        ) / 3.0D
    );

    int numberOfTrees = 0;
    if(this->rand->nextInt(10) == 0) {
        ++numberOfTrees;
    }

    switch(biome) {
        case BIOME_FOREST:
        case BIOME_RAINFOREST:
        case BIOME_TAIGA:
            numberOfTrees += treeDensitySample + 5;
            break;
        case BIOME_SEASONALFOREST:
            numberOfTrees += treeDensitySample + 2;
            break;
        case BIOME_DESERT:
        case BIOME_TUNDRA:
        case BIOME_PLAINS:
            numberOfTrees -= 20;
            break;
        default:
            break;
    }

    for(int i = 0; i < numberOfTrees; ++i) {
        xCoordinate = blockX + this->rand->nextInt(16) + 8;
        zCoordinate = blockZ + this->rand->nextInt(16) + 8;
        yCoordinate = world->GetHeightValue(xCoordinate, zCoordinate);

        enum TreeState {
            TREE_NONE,
            TREE_SMALL,
            TREE_BIG,
            TREE_BIRCH,
            TREE_TAIGA,
            TREE_TAIGA_ALT
        };

        TreeState ts = TREE_NONE;
        // Decide on a biome-appropriate tree
        switch(biome) {
            case BIOME_FOREST:
                if (rand->nextInt(5) == 0) {
                    ts = TREE_BIRCH;
                } else {
                    ts = (rand->nextInt(3) == 0) ? TREE_BIG : TREE_SMALL;
                }
                break;
            case BIOME_RAINFOREST:
                ts = (rand->nextInt(3) == 0) ? TREE_BIG : TREE_SMALL;
                break;
            case BIOME_TAIGA:
                ts = (rand->nextInt(3) == 0) ? TREE_TAIGA : TREE_TAIGA_ALT;
                break;
            default:
                ts = (rand->nextInt(10) == 0) ? TREE_BIG : TREE_SMALL;
                break;
        }

        // Generate the appropriate tree
        switch(ts) {
            case TREE_SMALL:
                Beta173Tree().Generate(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
                break;
            case TREE_BIRCH:
                Beta173Tree().Generate(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, true);
                break;
            case TREE_BIG:
                {
                    Beta173BigTree bt;
                    bt.Configure(1.0D, 1.0D, 1.0D);
                    bt.Generate(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
                    break;
                }
            case TREE_TAIGA:
                Beta173TaigaTree().Generate(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
                break;
            case TREE_TAIGA_ALT:
                Beta173TaigaAltTree().Generate(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
                break;
            default:
                break;
        }
    }

    int8_t numberOfFlowers = 0;
    switch (biome) {
        case BIOME_TAIGA:
        case BIOME_FOREST:
            numberOfFlowers = 2;
            break;
        case BIOME_SEASONALFOREST:
            numberOfFlowers = 4;
            break;
        case BIOME_PLAINS:
            numberOfFlowers = 3;
            break;
        default:
            break;
    }

    for(int8_t i = 0; i < numberOfFlowers; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature(BLOCK_DANDELION).GenerateFlowers(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    int8_t amountOfTallgrass = 0;
    switch(biome) {
        case BIOME_SEASONALFOREST:
        case BIOME_FOREST:
            amountOfTallgrass = 2;
            break;
        case BIOME_PLAINS:
        case BIOME_RAINFOREST:
            amountOfTallgrass = 10;
            break;
        case BIOME_TAIGA:
            amountOfTallgrass = 1;
            break;
        default:
            break;
    }

    for(int8_t i = 0; i < amountOfTallgrass; ++i) {
        // Normal Grass
        int8_t grassMeta = 1;
        if(
            biome == BIOME_RAINFOREST &&
            this->rand->nextInt(3) != 0
        ) {
            // Fern
            grassMeta = 2;
        }

        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature(BLOCK_TALLGRASS, grassMeta).GenerateTallgrass(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    int8_t numberOfDeadbushes = 0;
    if(biome == BIOME_DESERT) numberOfDeadbushes = 2;

    for(int i = 0; i < numberOfDeadbushes; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature(BLOCK_DEADBUSH).GenerateDeadbush(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    if(this->rand->nextInt(2) == 0) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature(BLOCK_ROSE).GenerateFlowers(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    if(this->rand->nextInt(4) == 0) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature(BLOCK_MUSHROOM_BROWN).GenerateFlowers(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    if(this->rand->nextInt(8) == 0) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature(BLOCK_MUSHROOM_RED).GenerateFlowers(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    for(int i = 0; i < 10; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature(BLOCK_SUGARCANE).GenerateSugarcane(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    if(this->rand->nextInt(32) == 0) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        Beta173Feature(BLOCK_SUGARCANE).GeneratePumpkins(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
    }

    int8_t numberOfCacti = 0;
    if(biome == BIOME_DESERT) {
        numberOfCacti += 10;
    }

    for(int i = 0; i < numberOfCacti; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        //(new WorldGenCactus()).generate(this->worldObj, this->rand, z, int offsetX, offsetZ);
    }

    for(int i = 0; i < 50; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(this->rand->nextInt(120) + 8);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        //(new WorldGenLiquids(Block.waterMoving.blockID)).generate(this->worldObj, this->rand, z, int offsetX, offsetZ);
    }

    for(int i = 0; i < 20; ++i) {
        xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
        yCoordinate = this->rand->nextInt(this->rand->nextInt(this->rand->nextInt(112) + 8) + 8);
        zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
        //(new WorldGenLiquids(Block.lavaMoving.blockID)).generate(this->worldObj, this->rand, z, int offsetX, offsetZ);
    }

	this->temperature = GenerateTemperature(
        blockX + 8,
        blockZ + 8,
        CHUNK_WIDTH_X,
        CHUNK_WIDTH_Z
    );

    // Place Snow in cold regions
    for(int x = blockX + 8; x < blockX + 8 + 16; ++x) {
        for(int z = blockZ + 8; z < blockZ + 8 + 16; ++z) {
            int offsetX = x - (blockX + 8);
            int offsetZ = z - (blockZ + 8);
            int highestBlock = world->GetHighestSolidOrLiquidBlock(x, z);
            double temp = this->temperature[offsetX * 16 + offsetZ] - (double)(highestBlock - 64) / 64.0D * 0.3D;
            if(temp < 0.5D &&
                highestBlock > 0 &&
                highestBlock < CHUNK_HEIGHT &&
                world->GetBlockType(Int3{x, highestBlock, z}) == BLOCK_AIR &&
                IsSolid(world->GetBlockType(Int3{x, highestBlock - 1, z})) &&
                world->GetBlockType(Int3{x, highestBlock - 1, z}) != BLOCK_ICE
            ) {
                world->SetBlockType(BLOCK_SNOW_LAYER, Int3{x, highestBlock, z});
            }
        }
    }

    //BlockSand.fallInstantly = false;
    return true;
}