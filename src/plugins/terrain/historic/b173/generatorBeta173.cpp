#include "generatorBeta173.h"

GeneratorBeta173::GeneratorBeta173(int64_t pSeed, World *pWorld) : Generator(pSeed, pWorld) {
	logger = &Betrock::Logger::Instance();
	this->seed = pSeed;
	this->world = pWorld;

	rand = std::make_unique<JavaRandom>(this->seed);

	// Init Terrain Noise
	lowNoiseGen = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
	highNoiseGen = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
	selectorNoiseGen = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);
	sandGravelNoiseGen = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
	stoneNoiseGen = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
	continentalnessNoiseGen = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 10);
	depthNoiseGen = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
	treeDensityNoiseGen = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);

	// Init Biome Noise
	auto randTemp = std::make_unique<JavaRandom>(this->seed * 9871L);
	auto randHum = std::make_unique<JavaRandom>(this->seed * 39811L);
	auto randWeird = std::make_unique<JavaRandom>(this->seed * 543321L);
	temperatureNoiseGen = std::make_unique<NoiseOctaves<NoiseSimplex>>(randTemp.get(), 4);
	humidityNoiseGen = std::make_unique<NoiseOctaves<NoiseSimplex>>(randHum.get(), 4);
	weirdnessNoiseGen = std::make_unique<NoiseOctaves<NoiseSimplex>>(randWeird.get(), 2);

	// Init Caver
	caver = std::make_unique<Beta173Caver>();
}

std::shared_ptr<Chunk> GeneratorBeta173::GenerateChunk(int32_t cX, int32_t cZ) {
	std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this->world, cX, cZ);
	this->rand->setSeed((long)cX * 341873128712L + (long)cZ * 132897987541L);

	// Allocate empty chunk
	c->ClearChunk();

	// Generate Biomes
	GenerateBiomeMap(cX * CHUNK_WIDTH_X, cZ * CHUNK_WIDTH_Z, CHUNK_WIDTH_X, CHUNK_WIDTH_Z);

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

	// Testing for pack.png seed
	// std::cout << std::hex;
	if (cX == 5 && cZ == -5) {
		// c->PrintHeightmap();
	}

	c->state = ChunkState::Generated;
	c->modified = true;
	return c;
}

// Replace some of the stone with Biome-appropriate blocks
void GeneratorBeta173::ReplaceBlocksForBiome(int cX, int cZ, std::shared_ptr<Chunk> &c) {
	const double oneThirtySecond = 1.0 / 32.0;
	// Init noise maps
	this->sandNoise.resize(256, 0.0);
	this->gravelNoise.resize(256, 0.0);
	this->stoneNoise.resize(256, 0.0);

	// Populate noise maps
	this->sandGravelNoiseGen->GenerateOctaves(this->sandNoise, (double)(cX * CHUNK_WIDTH_X),
											  (double)(cZ * CHUNK_WIDTH_Z), 0.0, 16, 16, 1, oneThirtySecond,
											  oneThirtySecond, 1.0);
	this->sandGravelNoiseGen->GenerateOctaves(this->gravelNoise, (double)(cX * CHUNK_WIDTH_X), 109.0134,
											  (double)(cZ * CHUNK_WIDTH_Z), 16, 1, 16, oneThirtySecond, 1.0,
											  oneThirtySecond);
	this->stoneNoiseGen->GenerateOctaves(this->stoneNoise, (double)(cX * CHUNK_WIDTH_X), (double)(cZ * CHUNK_WIDTH_Z),
										 0.0, 16, 16, 1, oneThirtySecond * 2.0, oneThirtySecond * 2.0,
										 oneThirtySecond * 2.0);

	// Iterate through entire chunk
	for (int x = 0; x < CHUNK_WIDTH_X; ++x) {
		for (int z = 0; z < CHUNK_WIDTH_Z; ++z) {
			// Get values from noise maps
			Biome biome = biomeMap[x + z * 16];
			bool sandActive = this->sandNoise[x + z * CHUNK_WIDTH_X] + this->rand->nextDouble() * 0.2 > 0.0;
			bool gravelActive = this->gravelNoise[x + z * CHUNK_WIDTH_X] + this->rand->nextDouble() * 0.2 > 3.0;
			int stoneActive =
				(int)(this->stoneNoise[x + z * CHUNK_WIDTH_X] / 3.0 + 3.0 + this->rand->nextDouble() * 0.25);
			int stoneDepth = -1;
			// Get biome-appropriate top and filler blocks
			uint8_t topBlock = GetTopBlock(biome);
			uint8_t fillerBlock = GetFillerBlock(biome);

			// Iterate over column top to bottom
			for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {
				int blockIndex = (z * CHUNK_WIDTH_X + x) * CHUNK_HEIGHT + y;
				// Place Bedrock at bottom with some randomness
				if (y <= 0 + this->rand->nextInt(5)) {
					c->SetBlockType(BLOCK_BEDROCK, BlockIndexToPosition(blockIndex));
					continue;
				}

				uint8_t currentBlock = c->GetBlockType(BlockIndexToPosition(blockIndex));
				// Ignore air
				if (currentBlock == BLOCK_AIR) {
					stoneDepth = -1;
					continue;
				}

				// If we counter stone, start replacing it
				if (currentBlock == BLOCK_STONE) {
					if (stoneDepth == -1) {
						if (stoneActive <= 0) {
							topBlock = BLOCK_AIR;
							fillerBlock = BLOCK_STONE;
						} else if (y >= WATER_LEVEL - 4 && y <= WATER_LEVEL + 1) {
							// If we're close to the water level, apply gravel and sand
							topBlock = GetTopBlock(biome);
							fillerBlock = GetFillerBlock(biome);

							if (gravelActive)
								topBlock = BLOCK_AIR;
							if (gravelActive)
								fillerBlock = BLOCK_GRAVEL;
							if (sandActive)
								topBlock = BLOCK_SAND;
							if (sandActive)
								fillerBlock = BLOCK_SAND;
						}

						// Add water if we're below water level
						if (y < WATER_LEVEL && topBlock == BLOCK_AIR) {
							topBlock = (uint8_t)BLOCK_WATER_STILL;
						}

						stoneDepth = stoneActive;
						if (y >= WATER_LEVEL - 1) {
							c->SetBlockType(topBlock, BlockIndexToPosition(blockIndex));
						} else {
							c->SetBlockType(fillerBlock, BlockIndexToPosition(blockIndex));
						}
					} else if (stoneDepth > 0) {
						--stoneDepth;
						c->SetBlockType(fillerBlock, BlockIndexToPosition(blockIndex));
						if (stoneDepth == 0 && fillerBlock == BLOCK_SAND) {
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
void GeneratorBeta173::GenerateTerrain(int cX, int cZ, std::shared_ptr<Chunk> &c) {
	const int xMax = CHUNK_WIDTH_X / 4 + 1;	   // 3
	const uint8_t yMax = CHUNK_HEIGHT / 8 + 1; // 14
	const int zMax = CHUNK_WIDTH_Z / 4 + 1;	   // 3

	// Generate 4x16x4 low resolution noise map
	this->GenerateTerrainNoise(this->terrainNoiseField, cX * 4, 0, cZ * 4, xMax, yMax, zMax);

	// Terrain noise is interpolated and only sampled every 4 blocks
	for (int macroX = 0; macroX < 4; ++macroX) {
		for (int macroZ = 0; macroZ < 4; ++macroZ) {
			for (int macroY = 0; macroY < 16; ++macroY) {
				double verticalLerpStep = 0.125;

				// Get noise cube corners
				double corner000 = this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 0) * yMax + macroY + 0];
				double corner010 = this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 1) * yMax + macroY + 0];
				double corner100 = this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 0) * yMax + macroY + 0];
				double corner110 = this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 1) * yMax + macroY + 0];
				double corner001 =
					(this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 0) * yMax + macroY + 1] - corner000) *
					verticalLerpStep;
				double corner011 =
					(this->terrainNoiseField[((macroX + 0) * zMax + macroZ + 1) * yMax + macroY + 1] - corner010) *
					verticalLerpStep;
				double corner101 =
					(this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 0) * yMax + macroY + 1] - corner100) *
					verticalLerpStep;
				double corner111 =
					(this->terrainNoiseField[((macroX + 1) * zMax + macroZ + 1) * yMax + macroY + 1] - corner110) *
					verticalLerpStep;

				// Interpolate the 1/4th scale noise
				for (int subY = 0; subY < 8; ++subY) {
					double horizontalLerpStep = 0.25;
					double terrainX0 = corner000;
					double terrainX1 = corner010;
					double terrainStepX0 = (corner100 - corner000) * horizontalLerpStep;
					double terrainStepX1 = (corner110 - corner010) * horizontalLerpStep;

					for (int subX = 0; subX < 4; ++subX) {
						int blockIndex = ((subX + macroX * 4) << 11) | ((macroZ * 4) << 7) | ((macroY * 8) + subY);
						double terrainDensity = terrainX0;
						double densityStepZ = (terrainX1 - terrainX0) * horizontalLerpStep;

						for (int subZ = 0; subZ < 4; ++subZ) {
							// Here the actual block is determined
							// Default to air block
							uint8_t blockType = BLOCK_AIR;

							// If water is too cold, turn into ice
							double temp = this->temperature[(macroX * 4 + subX) * 16 + macroZ * 4 + subZ];
							int yLevel = macroY * 8 + subY;
							if (yLevel < WATER_LEVEL) {
								if (temp < 0.5 && yLevel >= WATER_LEVEL - 1) {
									blockType = BLOCK_ICE;
								} else {
									blockType = BLOCK_WATER_STILL;
								}
							}

							// If the terrain density falls below,
							// replace block with stone
							if (terrainDensity > 0.0) {
								blockType = BLOCK_STONE;
							}

							c->SetBlockType(blockType, BlockIndexToPosition(blockIndex));
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

void GeneratorBeta173::GenerateTemperature(int bx, int bz, int xMax, int zMax) {
	if (this->temperature.empty() || this->temperature.size() < size_t(xMax * zMax)) {
		this->temperature.resize(xMax * zMax, 0.0);
	}

	this->temperatureNoiseGen->GenerateOctaves(this->temperature, (double)bx, (double)bz, xMax, zMax, (double)0.025F,
											   (double)0.025F, 0.25);
	this->weirdnessNoiseGen->GenerateOctaves(this->weirdness, (double)bx, (double)bz, xMax, zMax, 0.25, 0.25,
											 0.5882352941176471);
	int index = 0;

	for (int x = 0; x < xMax; ++x) {
		for (int z = 0; z < zMax; ++z) {
			double var9 = this->weirdness[index] * 1.1 + 0.5;
			double scale = 0.01;
			double max = 1.0 - scale;
			double temp = (this->temperature[index] * 0.15 + 0.7) * max + var9 * scale;
			temp = 1.0 - (1.0 - temp) * (1.0 - temp);
			if (temp < 0.0) {
				temp = 0.0;
			}

			if (temp > 1.0) {
				temp = 1.0;
			}

			this->temperature[index] = temp;
			++index;
		}
	}
}

// Generate Biomes based on simplex noise
void GeneratorBeta173::GenerateBiomeMap(int bx, int bz, int xMax, int zMax) {
	// Init Biome map
	if (this->biomeMap.empty() || int(this->biomeMap.size()) < xMax * zMax) {
		this->biomeMap.resize(xMax * zMax, BIOME_NONE);
	}

	// Get noise values
	// We found an oversight in the original code! zMax is NEVER used for getting the noise range!
	// Although this is irrelevant for all intents and purposes, as xMax always equals zMax
	this->temperatureNoiseGen->GenerateOctaves(this->temperature, (double)bx, (double)bz, xMax, xMax, 0.025, 0.025,
											   0.25);
	this->humidityNoiseGen->GenerateOctaves(this->humidity, (double)bx, (double)bz, xMax, xMax, 0.05, 0.05, 1.0 / 3.0);
	this->weirdnessNoiseGen->GenerateOctaves(this->weirdness, (double)bx, (double)bz, xMax, xMax, 0.25, 0.25,
											 0.5882352941176471);
	int index = 0;

	// Iterate over each block column
	for (int iX = 0; iX < xMax; ++iX) {
		for (int iZ = 0; iZ < zMax; ++iZ) {
			double weird = this->weirdness[index] * 1.1 + 0.5;
			double scale = 0.01;
			double max = 1.0 - scale;
			double temp = (this->temperature[index] * 0.15 + 0.7) * max + weird * scale;
			scale = 0.002;
			max = 1.0 - scale;
			double humi = (this->humidity[index] * 0.15 + 0.5) * max + weird * scale;
			temp = 1.0 - (1.0 - temp) * (1.0 - temp);
			// Limit values to 0.0 - 1.0
			if (temp < 0.0)
				temp = 0.0;
			if (humi < 0.0)
				humi = 0.0;
			if (temp > 1.0)
				temp = 1.0;
			if (humi > 1.0)
				humi = 1.0;

			// Write the temperature and humidity values back
			this->temperature[index] = temp;
			this->humidity[index] = humi;
			// Get the biome from the lookup
			this->biomeMap[index] = GetBiomeFromLookup(temp, humi);
			index++;
		}
	}
}

// Make terrain noise
void GeneratorBeta173::GenerateTerrainNoise(std::vector<double> &terrainMap, int cX, int cY, int cZ, int xMax, int yMax,
											int zMax) {
	terrainMap.resize(xMax * yMax * zMax, 0.0);

	double horiScale = 684.412;
	double vertScale = 684.412;

	// We do this to need to generate noise as often
	this->continentalnessNoiseGen->GenerateOctaves(this->continentalnessNoiseField, cX, cZ, xMax, zMax, 1.121, 1.121,
												   0.5);
	this->depthNoiseGen->GenerateOctaves(this->depthNoiseField, cX, cZ, xMax, zMax, 200.0, 200.0, 0.5);
	this->selectorNoiseGen->GenerateOctaves(this->selectorNoiseField, (double)cX, (double)cY, (double)cZ, xMax, yMax,
											zMax, horiScale / 80.0, vertScale / 160.0, horiScale / 80.0);
	this->lowNoiseGen->GenerateOctaves(this->lowNoiseField, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax,
									   horiScale, vertScale, horiScale);
	this->highNoiseGen->GenerateOctaves(this->highNoiseField, (double)cX, (double)cY, (double)cZ, xMax, yMax, zMax,
										horiScale, vertScale, horiScale);
	// Used to iterate 3D noise maps (low, high, selector)
	int xyzIndex = 0;
	// Used to iterate 2D Noise maps (depth, continentalness)
	int xzIndex = 0;
	int scaleFraction = 16 / xMax;

	for (int iX = 0; iX < xMax; ++iX) {
		int sampleX = iX * scaleFraction + scaleFraction / 2;

		for (int iZ = 0; iZ < zMax; ++iZ) {
			// Sample 2D noises
			int sampleZ = iZ * scaleFraction + scaleFraction / 2;
			// Apply biome-noise-dependent variety
			double temp = this->temperature[sampleX * CHUNK_WIDTH_X + sampleZ];
			double humi = this->humidity[sampleX * CHUNK_WIDTH_X + sampleZ] * temp;
			humi = 1.0 - humi;
			humi *= humi;
			humi *= humi;
			humi = 1.0 - humi;
			// Apply contientalness
			double continentalness = (this->continentalnessNoiseField[xzIndex] + 256.0) / 512.0;
			continentalness *= humi;
			if (continentalness > 1.0)
				continentalness = 1.0;

			double depthNoise = this->depthNoiseField[xzIndex] / 8000.0;
			if (depthNoise < 0.0)
				depthNoise = -depthNoise * 0.3;

			depthNoise = depthNoise * 3.0 - 2.0;
			if (depthNoise < 0.0) {
				depthNoise /= 2.0;
				if (depthNoise < -1.0)
					depthNoise = -1.0;

				depthNoise /= 1.4;
				depthNoise /= 2.0;
				continentalness = 0.0;
			} else {
				if (depthNoise > 1.0)
					depthNoise = 1.0;
				depthNoise /= 8.0;
			}

			if (continentalness < 0.0) {
				continentalness = 0.0;
			}

			continentalness += 0.5;
			depthNoise = depthNoise * (double)yMax / 16.0;
			double elevationOffset = (double)yMax / 2.0 + depthNoise * 4.0;
			++xzIndex;

			for (int iY = 0; iY < yMax; ++iY) {
				// Sample 3D noises
				double terrainDensity = 0.0;
				double densityOffset = ((double)iY - elevationOffset) * 12.0 / continentalness;
				if (densityOffset < 0.0) {
					densityOffset *= 4.0;
				}

				double lowNoise = this->lowNoiseField[xyzIndex] / 512.0;
				double highNoise = this->highNoiseField[xyzIndex] / 512.0;
				double selectorNoise = (this->selectorNoiseField[xyzIndex] / 10.0 + 1.0) / 2.0;
				if (selectorNoise < 0.0) {
					terrainDensity = lowNoise;
				} else if (selectorNoise > 1.0) {
					terrainDensity = highNoise;
				} else {
					terrainDensity = lowNoise + (highNoise - lowNoise) * selectorNoise;
				}

				terrainDensity -= densityOffset;
				// Reduce density towards max height
				if (iY > yMax - 4) {
					double heightEdgeFade = (double)((float)(iY - (yMax - 4)) / 3.0F);
					terrainDensity = terrainDensity * (1.0 - heightEdgeFade) + -10.0 * heightEdgeFade;
				}

				terrainMap[xyzIndex] = terrainDensity;
				++xyzIndex;
			}
		}
	}
}

Biome GeneratorBeta173::GetBiomeAt(int worldX, int worldZ) {
	int localX = worldX % CHUNK_WIDTH_X;
	int localZ = worldZ % CHUNK_WIDTH_Z;
	if (localX < 0)
		localX += CHUNK_WIDTH_X;
	if (localZ < 0)
		localZ += CHUNK_WIDTH_Z;
	return biomeMap[localX + localZ * CHUNK_WIDTH_X];
}

bool GeneratorBeta173::PopulateChunk(int32_t cX, int32_t cZ) {
	// BlockSand.fallInstantly = true;
	int blockX = cX * CHUNK_WIDTH_X;
	int blockZ = cZ * CHUNK_WIDTH_Z;
	Biome biome = GetBiomeAt(blockX + 16, blockZ + 16);
	this->rand->setSeed(this->world->seed);
	long xOffset = this->rand->nextLong() / 2L * 2L + 1L;
	long zOffset = this->rand->nextLong() / 2L * 2L + 1L;
	this->rand->setSeed(((long(cX) * xOffset) + (long(cZ) * zOffset)) ^ this->world->seed);
	[[maybe_unused]] int xCoordinate;
	[[maybe_unused]] int yCoordinate;
	[[maybe_unused]] int zCoordinate;

	// Generate lakes
	if (this->rand->nextInt(4) == 0) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_WATER_STILL)
			.GenerateLake(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	// Generate lava lakes
	if (this->rand->nextInt(8) == 0) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(this->rand->nextInt(120) + 8);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		if (yCoordinate < WATER_LEVEL || this->rand->nextInt(10) == 0) {
			Beta173Feature(BLOCK_LAVA_STILL)
				.GenerateLake(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
		}
	}

	for (int i = 0; i < 8; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature().GenerateDungeon(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	for (int i = 0; i < 10; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
		Beta173Feature().GenerateClay(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 32);
	}

	for (int i = 0; i < 20; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
		Beta173Feature(BLOCK_DIRT)
			.GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 32);
	}

	for (int i = 0; i < 10; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
		Beta173Feature(BLOCK_GRAVEL)
			.GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 32);
	}

	for (int i = 0; i < 20; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
		Beta173Feature(BLOCK_ORE_COAL)
			.GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 16);
	}

	for (int i = 0; i < 20; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT / 2);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
		Beta173Feature(BLOCK_ORE_IRON)
			.GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 8);
	}

	for (int i = 0; i < 2; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT / 4);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
		Beta173Feature(BLOCK_ORE_GOLD)
			.GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 8);
	}

	for (int i = 0; i < 8; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT / 8);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
		Beta173Feature(BLOCK_ORE_REDSTONE_OFF)
			.GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 7);
	}

	for (int i = 0; i < 1; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT / 8);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
		Beta173Feature(BLOCK_ORE_DIAMOND)
			.GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 7);
	}

	for (int i = 0; i < 1; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X);
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT / 8) + this->rand->nextInt(16);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z);
		Beta173Feature(BLOCK_ORE_LAPIS_LAZULI)
			.GenerateMinable(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, 6);
	}

	double fraction = 0.5;
	int treeDensitySample =
		int((this->treeDensityNoiseGen->GenerateOctaves(double(blockX) * fraction, double(blockZ) * fraction) / 8.0 +
			 this->rand->nextDouble() * 4.0 + 4.0) /
			3.0);

	int numberOfTrees = 0;
	if (this->rand->nextInt(10) == 0) {
		++numberOfTrees;
	}

	switch (biome) {
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

	for (int i = 0; i < numberOfTrees; ++i) {
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
		switch (biome) {
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
		switch (ts) {
		case TREE_SMALL:
			Beta173Tree().Generate(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
			break;
		case TREE_BIRCH:
			Beta173Tree().Generate(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate, true);
			break;
		case TREE_BIG: {
			Beta173BigTree bt;
			bt.Configure(1.0, 1.0, 1.0);
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

	for (int8_t i = 0; i < numberOfFlowers; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_DANDELION)
			.GenerateFlowers(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	int8_t amountOfTallgrass = 0;
	switch (biome) {
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

	for (int8_t i = 0; i < amountOfTallgrass; ++i) {
		// Normal Grass
		int8_t grassMeta = 1;
		if (biome == BIOME_RAINFOREST && this->rand->nextInt(3) != 0) {
			// Fern
			grassMeta = 2;
		}

		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_TALLGRASS, grassMeta)
			.GenerateTallgrass(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	int8_t numberOfDeadbushes = 0;
	if (biome == BIOME_DESERT)
		numberOfDeadbushes = 2;

	for (int i = 0; i < numberOfDeadbushes; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_DEADBUSH)
			.GenerateDeadbush(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	if (this->rand->nextInt(2) == 0) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_ROSE)
			.GenerateFlowers(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	if (this->rand->nextInt(4) == 0) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_MUSHROOM_BROWN)
			.GenerateFlowers(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	if (this->rand->nextInt(8) == 0) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_MUSHROOM_RED)
			.GenerateFlowers(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	for (int i = 0; i < 10; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_SUGARCANE)
			.GenerateSugarcane(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	if (this->rand->nextInt(32) == 0) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_PUMPKIN)
			.GeneratePumpkins(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	int8_t numberOfCacti = 0;
	if (biome == BIOME_DESERT) {
		numberOfCacti += 10;
	}

	for (int i = 0; i < numberOfCacti; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(CHUNK_HEIGHT);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_CACTUS)
			.GenerateCacti(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	for (int i = 0; i < 50; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(this->rand->nextInt(120) + 8);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_WATER_FLOWING)
			.GenerateLiquid(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	for (int i = 0; i < 20; ++i) {
		xCoordinate = blockX + this->rand->nextInt(CHUNK_WIDTH_X) + 8;
		yCoordinate = this->rand->nextInt(this->rand->nextInt(this->rand->nextInt(112) + 8) + 8);
		zCoordinate = blockZ + this->rand->nextInt(CHUNK_WIDTH_Z) + 8;
		Beta173Feature(BLOCK_LAVA_FLOWING)
			.GenerateLiquid(this->world, this->rand.get(), xCoordinate, yCoordinate, zCoordinate);
	}

	GenerateTemperature(blockX + 8, blockZ + 8, CHUNK_WIDTH_X, CHUNK_WIDTH_Z);

	// Place Snow in cold regions
	for (int x = blockX + 8; x < blockX + 8 + 16; ++x) {
		for (int z = blockZ + 8; z < blockZ + 8 + 16; ++z) {
			int offsetX = x - (blockX + 8);
			int offsetZ = z - (blockZ + 8);
			int highestBlock = world->GetHighestSolidOrLiquidBlock(x, z);
			double temp = this->temperature[offsetX * 16 + offsetZ] - (double)(highestBlock - 64) / 64.0 * 0.3;
			if (temp < 0.5 && highestBlock > 0 && highestBlock < CHUNK_HEIGHT &&
				world->GetBlockType(Int3{x, highestBlock, z}) == BLOCK_AIR &&
				IsSolid(world->GetBlockType(Int3{x, highestBlock - 1, z})) &&
				world->GetBlockType(Int3{x, highestBlock - 1, z}) != BLOCK_ICE) {
				world->SetBlockType(BLOCK_SNOW_LAYER, Int3{x, highestBlock, z});
			}
		}
	}

	// BlockSand.fallInstantly = false;
	return true;
}