#include "generatorAlpha112_01.h"

/**
 * @brief Construct a new Beta 1.7.3 Generator
 * 
 * @param pSeed The seed of the generated world
 * @param pWorld The world that the generator belongs to
 */
GeneratorAlpha112_01::GeneratorAlpha112_01(int64_t pSeed, World *pWorld) : Generator(pSeed, pWorld) {
	logger = &Betrock::Logger::Instance();
	this->seed = pSeed;
	this->world = pWorld;

	rand = JavaRandom(this->seed);

	// Init Terrain Noise
	lowNoiseGen = NoiseOctaves<NoisePerlin>(rand, 16);
	highNoiseGen = NoiseOctaves<NoisePerlin>(rand, 16);
	selectorNoiseGen = NoiseOctaves<NoisePerlin>(rand, 8);
	sandGravelNoiseGen = NoiseOctaves<NoisePerlin>(rand, 4);
	stoneNoiseGen = NoiseOctaves<NoisePerlin>(rand, 4);
	continentalnessNoiseGen = NoiseOctaves<NoisePerlin>(rand, 10);
	depthNoiseGen = NoiseOctaves<NoisePerlin>(rand, 16);
	treeDensityNoiseGen = NoiseOctaves<NoisePerlin>(rand, 8);

	// Init Caver
	caver = std::make_unique<Beta173Caver>();
}

/**
 * @brief Generate a non-populated chunk
 * 
 * @param chunkPos The x,z coordinate of the chunk
 * @return std::shared_ptr<Chunk> 
 */
std::shared_ptr<Chunk> GeneratorAlpha112_01::GenerateChunk(Int2 chunkPos) {
	std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this->world, chunkPos);
	c->state = ChunkState::Generating;
	this->rand.setSeed(int64_t(chunkPos.x) * 341873128712L + int64_t(chunkPos.y) * 132897987541L);
	// Allocate empty chunk
	c->ClearChunk();
	// Generate the Terrain, minus any caves, as just stone
	GenerateTerrain(chunkPos, c);
	// Replace some of the stone with surface-appropriate blocks
	ReplaceBlocksForBiome(chunkPos, c);
	// Carve caves
	caver->CarveCavesForChunk(this->world, chunkPos, c);
	// Generate heightmap
	c->GenerateHeightMap();
	// Try to populate
	//c->PopulateChunk(chunkPos);
	c->state = ChunkState::Generated;
	c->modified = true;
	return c;
}

/**
 * @brief Replace some of the stone with Biome-appropriate blocks.
 *        Top block is always grass, filler is always dirt (matching Java).
 * 
 * @param chunkPos The x,z coordinate of the chunk
 * @param c The chunk that should get its blocks replaced
 */
void GeneratorAlpha112_01::ReplaceBlocksForBiome(Int2 chunkPos, std::shared_ptr<Chunk> &c) {
	const double oneThirtySecond = 1.0 / 32.0;

	// Init noise maps
	this->sandNoise.resize(256, 0.0);
	this->gravelNoise.resize(256, 0.0);
	this->stoneNoise.resize(256, 0.0);

	// Populate noise maps
	this->sandGravelNoiseGen.GenerateOctaves(this->sandNoise, double(chunkPos.x * CHUNK_WIDTH_X),
											  double(chunkPos.y * CHUNK_WIDTH_Z), 0.0, 16, 16, 1, oneThirtySecond,
											  oneThirtySecond, 1.0);
	this->sandGravelNoiseGen.GenerateOctaves(this->gravelNoise, double(chunkPos.x * CHUNK_WIDTH_X), 109.0134,
											  double(chunkPos.y * CHUNK_WIDTH_Z), 16, 1, 16, oneThirtySecond, 1.0,
											  oneThirtySecond);
	this->stoneNoiseGen.GenerateOctaves(this->stoneNoise, double(chunkPos.x * CHUNK_WIDTH_X), double(chunkPos.y * CHUNK_WIDTH_Z),
										 0.0, 16, 16, 1, oneThirtySecond * 2.0, oneThirtySecond * 2.0,
										 oneThirtySecond * 2.0);

	// Iterate through entire chunk
	for (int32_t x = 0; x < CHUNK_WIDTH_X; ++x) {
		for (int32_t z = 0; z < CHUNK_WIDTH_Z; ++z) {
			// Get values from noise maps
			bool sandActive   = this->sandNoise[x + z * CHUNK_WIDTH_X] + this->rand.nextDouble() * 0.2 > 0.0;
			bool gravelActive = this->gravelNoise[x + z * CHUNK_WIDTH_X] + this->rand.nextDouble() * 0.2 > 3.0;
			int32_t stoneActive =
				Java::DoubleToInt32(this->stoneNoise[x + z * CHUNK_WIDTH_X] / 3.0 + 3.0 + this->rand.nextDouble() * 0.25);
			int32_t stoneDepth = -1;

			// Java always uses grass as top block and dirt as filler — no biome lookup
			BlockType topBlock    = BLOCK_GRASS;
			BlockType fillerBlock = BLOCK_DIRT;

			// Iterate over column top to bottom
			for (int32_t y = CHUNK_HEIGHT - 1; y >= 0; --y) {
				int32_t blockIndex = (z * CHUNK_WIDTH_X + x) * CHUNK_HEIGHT + y;

				// FIX: Match Java's bedrock roll: nextInt(6) - 1, range [-1, 4]
				if (y <= 0 + this->rand.nextInt(6) - 1) {
					c->SetBlockType(BLOCK_BEDROCK, BlockIndexToPosition(blockIndex));
					continue;
				}

				BlockType currentBlock = c->GetBlockType(BlockIndexToPosition(blockIndex));

				// Ignore air
				if (currentBlock == BLOCK_AIR) {
					stoneDepth = -1;
					continue;
				}

				// If we encounter stone, start replacing it
				if (currentBlock == BLOCK_STONE) {
					if (stoneDepth == -1) {
						if (stoneActive <= 0) {
							topBlock    = BLOCK_AIR;
							fillerBlock = BLOCK_STONE;
						} else if (y >= WATER_LEVEL - 4 && y <= WATER_LEVEL + 1) {
							// If we're close to the water level, apply gravel and sand
							topBlock    = BLOCK_GRASS;
							fillerBlock = BLOCK_DIRT;

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
							topBlock = BLOCK_WATER_STILL;
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
						// FIX: Removed sandstone generation — not present in Java
					}
				}
			}
		}
	}
}

/**
 * @brief Generate the Terrain, minus any caves, as just stone
 * 
 * @param chunkPos The x,z coordinate of the chunk
 * @param c The chunk that should get its terrain generated
 */
void GeneratorAlpha112_01::GenerateTerrain(Int2 chunkPos, std::shared_ptr<Chunk> &c) {
	const int32_t xMax = CHUNK_WIDTH_X / 4 + 1; // 5
	const uint8_t yMax = CHUNK_HEIGHT / 8 + 1;  // 17
	const int32_t zMax = CHUNK_WIDTH_Z / 4 + 1; // 5

	// Generate low-resolution noise map
	this->GenerateTerrainNoise(this->terrainNoiseField, Int3{chunkPos.x * 4, 0, chunkPos.y * 4}, Int3{xMax, yMax, zMax});

	// Terrain noise is interpolated and only sampled every 4 blocks
	for (int32_t macroX = 0; macroX < 4; ++macroX) {
		for (int32_t macroZ = 0; macroZ < 4; ++macroZ) {
			for (int32_t macroY = 0; macroY < 16; ++macroY) {
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
				for (int32_t subY = 0; subY < 8; ++subY) {
					double horizontalLerpStep = 0.25;
					double terrainX0 = corner000;
					double terrainX1 = corner010;
					double terrainStepX0 = (corner100 - corner000) * horizontalLerpStep;
					double terrainStepX1 = (corner110 - corner010) * horizontalLerpStep;

					for (int32_t subX = 0; subX < 4; ++subX) {
						int32_t blockIndex = ((subX + macroX * 4) << 11) | ((macroZ * 4) << 7) | ((macroY * 8) + subY);
						double terrainDensity = terrainX0;
						double densityStepZ = (terrainX1 - terrainX0) * horizontalLerpStep;

						for (int32_t subZ = 0; subZ < 4; ++subZ) {
							BlockType blockType = BLOCK_AIR;

							int32_t yLevel = macroY * 8 + subY;

							// FIX: Match Java — use worldObj.snowCovered flag for ice,
							//      not a per-column temperature check.
							if (yLevel < WATER_LEVEL) {
								if (this->snowCovered && yLevel >= WATER_LEVEL - 1) {
									blockType = BLOCK_ICE;
								} else {
									blockType = BLOCK_WATER_STILL;
								}
							}

							if (terrainDensity > 0.0) {
								blockType = BLOCK_STONE;
							}

							c->SetBlockType(blockType, BlockIndexToPosition(blockIndex));
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

/**
 * @brief Make terrain noise and updates the terrain map.
 *        Matches Java's initializeNoiseField exactly.
 * 
 * @param terrainMap The terrain map that the scaled-down terrain values will be written to
 * @param chunkPos   The x,y,z coordinate of the sub-chunk
 * @param max        Defines the area of the terrainMap
 */
void GeneratorAlpha112_01::GenerateTerrainNoise(std::vector<double> &terrainMap, Int3 chunkPos, Int3 max) {
	terrainMap.resize(max.x * max.y * max.z, 0.0);

	const double horiScale = 684.412;
	const double vertScale = 684.412;

	// FIX: Match Java call signatures exactly.
	// noiseGen6 (continentalness): x/z only (Y=0), scales 1.0, 0.0, 1.0
	this->continentalnessNoiseGen.GenerateOctaves(
		this->continentalnessNoiseField,
		(double)chunkPos.x, 0.0, (double)chunkPos.z,
		max.x, 1, max.z,
		1.0, 0.0, 1.0);

	// noiseGen7 (depth): x/z only (Y=0), scales 100.0, 0.0, 100.0
	this->depthNoiseGen.GenerateOctaves(
		this->depthNoiseField,
		(double)chunkPos.x, 0.0, (double)chunkPos.z,
		max.x, 1, max.z,
		100.0, 0.0, 100.0);

	this->selectorNoiseGen.GenerateOctaves(this->selectorNoiseField,
		(double)chunkPos.x, (double)chunkPos.y, (double)chunkPos.z,
		max.x, max.y, max.z,
		horiScale / 80.0, vertScale / 160.0, horiScale / 80.0);
	this->lowNoiseGen.GenerateOctaves(this->lowNoiseField,
		(double)chunkPos.x, (double)chunkPos.y, (double)chunkPos.z,
		max.x, max.y, max.z,
		horiScale, vertScale, horiScale);
	this->highNoiseGen.GenerateOctaves(this->highNoiseField,
		(double)chunkPos.x, (double)chunkPos.y, (double)chunkPos.z,
		max.x, max.y, max.z,
		horiScale, vertScale, horiScale);

	// Used to iterate 3D noise maps (low, high, selector)
	int32_t xyzIndex = 0;
	// Used to iterate 2D noise maps (depth, continentalness)
	int32_t xzIndex = 0;

	for (int32_t iX = 0; iX < max.x; ++iX) {
		for (int32_t iZ = 0; iZ < max.z; ++iZ) {
			// FIX: No humidity/temperature influence on noise — Java has none here.
			double continentalness = (this->continentalnessNoiseField[xzIndex] + 256.0) / 512.0;
			if (continentalness > 1.0)
				continentalness = 1.0;

			double depthNoise = this->depthNoiseField[xzIndex] / 8000.0;
			if (depthNoise < 0.0)
				depthNoise = -depthNoise; // take absolute value (Java: var20 = -var20)

			// FIX 1: Java uses * 3.0 - 3.0, not * 3.0 - 2.0
			depthNoise = depthNoise * 3.0 - 3.0;

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
				// FIX 2: Java divides by 6.0, not 8.0
				depthNoise /= 6.0;
			}

			continentalness += 0.5;
			depthNoise = depthNoise * double(max.y) / 16.0;
			double elevationOffset = double(max.y) / 2.0 + depthNoise * 4.0;
			++xzIndex;

			for (int32_t iY = 0; iY < max.y; ++iY) {
				double terrainDensity = 0.0;
				double densityOffset  = ((double)iY - elevationOffset) * 12.0 / continentalness;
				if (densityOffset < 0.0)
					densityOffset *= 4.0;

				double lowNoise      = this->lowNoiseField[xyzIndex] / 512.0;
				double highNoise     = this->highNoiseField[xyzIndex] / 512.0;
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
				if (iY > max.y - 4) {
					double heightEdgeFade = double(float(iY - (max.y - 4)) / 3.0F);
					terrainDensity = (terrainDensity * (1.0 - heightEdgeFade)) + (-10.0 * heightEdgeFade);
				}

				terrainMap[xyzIndex] = terrainDensity;
				++xyzIndex;
			}
		}
	}
}

/**
 * @brief Populates the specified chunk with biome-specific features
 * 
 * @param chunkPos The x,z coordinate of the chunk
 * @return True if population succeeded
 */
bool GeneratorAlpha112_01::PopulateChunk(Int2 chunkPos) {
	return true;
}