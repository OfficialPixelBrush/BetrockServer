#include "generatorInfdev20100327.h"

GeneratorInfdev20100327::GeneratorInfdev20100327(int64_t pSeed, World *pWorld) : Generator(pSeed, pWorld) {
	logger = &Betrock::Logger::Instance();
	this->seed = pSeed;
	this->world = pWorld;

	rand = JavaRandom(this->seed);
	noiseGen1 = NoiseOctaves<NoisePerlin>(rand, 16);
	noiseGen2 = NoiseOctaves<NoisePerlin>(rand, 16);
	noiseGen3 = NoiseOctaves<NoisePerlin>(rand, 8);
	noiseGen4 = NoiseOctaves<NoisePerlin>(rand, 4);
	noiseGen5 = NoiseOctaves<NoisePerlin>(rand, 4);
	noiseGen6 = NoiseOctaves<NoisePerlin>(rand, 5);
	mobSpawnerNoise = NoiseOctaves<NoisePerlin>(rand, 5);
}

std::shared_ptr<Chunk> GeneratorInfdev20100327::GenerateChunk(Int2 chunkPos) {
	std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this->world, chunkPos);
	c->state = ChunkState::Generating;
	rand.setSeed(int64_t(chunkPos.x) * 341873128712L + int64_t(chunkPos.y) * 132897987541L);
	c->ClearChunk();

	// Terrain shape generation
	for (int32_t macroX = 0; macroX < 4; ++macroX) {
		for (int32_t macroZ = 0; macroZ < 4; ++macroZ) {
			double var7[33][4];
			int32_t macroY = (chunkPos.x << 2) + macroX;
			int32_t blockY = (chunkPos.y << 2) + macroZ;

			// 33 == size of var7's first
			for (int32_t var10 = 0; var10 < 33; ++var10) {
				var7[var10][0] = this->InitializeNoiseField((double)macroY, (double)var10, (double)blockY);
				var7[var10][1] = this->InitializeNoiseField((double)macroY, (double)var10, double(blockY + 1));
				var7[var10][2] = this->InitializeNoiseField(double(macroY + 1), (double)var10, (double)blockY);
				var7[var10][3] = this->InitializeNoiseField(double(macroY + 1), (double)var10, double(blockY + 1));
			}

			for (macroY = 0; macroY < 32; ++macroY) {
				double macroX0 = var7[macroY][0];
				double var11 = var7[macroY][1];
				double var13 = var7[macroY][2];
				double var15 = var7[macroY][3];
				double var17 = var7[macroY + 1][0];
				double var19 = var7[macroY + 1][1];
				double var21 = var7[macroY + 1][2];
				double var23 = var7[macroY + 1][3];

				for (int32_t var25 = 0; var25 < 4; ++var25) {
					double var26 = (double)var25 / 4.0;
					double var28 = macroX0 + (var17 - macroX0) * var26;
					double var30 = var11 + (var19 - var11) * var26;
					double var32 = var13 + (var21 - var13) * var26;
					double var34 = var15 + (var23 - var15) * var26;
					for (int32_t macroX1 = 0; macroX1 < 4; ++macroX1) {
						double var37 = (double)macroX1 / 4.0;
						double var39 = var28 + (var32 - var28) * var37;
						double var41 = var30 + (var34 - var30) * var37;
						int32_t blockIndex =
							((macroX1 + (macroX << 2)) << 11) | ((0 + (macroZ << 2)) << 7) | ((macroY << 2) + var25);

						for (int32_t var36 = 0; var36 < 4; ++var36) {
							double var45 = (double)var36 / 4.0;
							double terrainDensity = var39 + (var41 - var39) * var45;
							BlockType blockType = BLOCK_AIR;
							if ((macroY << 2) + var25 < WATER_LEVEL) {
								blockType = BLOCK_WATER_STILL;
							}

							if (terrainDensity > 0.0) {
								blockType = BLOCK_STONE;
							}

							c->SetBlockType(blockType, BlockIndexToPosition(blockIndex));
							blockIndex += CHUNK_HEIGHT;
						}
					}
				}
			}
		}
	}

	// "Biome" blocks
	for (int32_t blockX = 0; blockX < CHUNK_WIDTH_X; ++blockX) {
		for (int32_t blockZ = 0; blockZ < CHUNK_WIDTH_Z; ++blockZ) {
			int32_t blockIndex = blockX << 11 | blockZ << 7 | 127;
			int32_t depth = -1;

			for (int32_t blockY = CHUNK_HEIGHT - 1; blockY >= 0; --blockY) {
				if (c->GetBlockType(BlockIndexToPosition(blockIndex)) == BLOCK_AIR) {
					depth = -1;
				} else if (c->GetBlockType(BlockIndexToPosition(blockIndex)) == BLOCK_STONE) {
					if (depth == -1) {
						depth = 3;
						if (blockY >= WATER_LEVEL - 1) {
							c->SetBlockType(BLOCK_GRASS, BlockIndexToPosition(blockIndex));
						} else {
							c->SetBlockType(BLOCK_DIRT, BlockIndexToPosition(blockIndex));
						}
					} else if (depth > 0) {
						--depth;
						c->SetBlockType(BLOCK_DIRT, BlockIndexToPosition(blockIndex));
					}
				}

				--blockIndex;
			}
		}
	}

	c->GenerateHeightMap();
	c->state = ChunkState::Generated;
	c->modified = true;
	return c;
}

double GeneratorInfdev20100327::InitializeNoiseField(double var1, double var3, double macroX) {
	double var7 = var3 * 4.0 - 64.0;
	if (var7 < 0.0) {
		var7 *= 3.0;
	}

	double blockY =
		this->noiseGen3.GenerateOctaves(var1 * 684.412 / 80.0, var3 * 684.412 / 400.0, macroX * 684.412 / 80.0) / 2.0;
	double var11;
	double var13;
	if (blockY < -1.0) {
		var11 = this->noiseGen1.GenerateOctaves(var1 * 684.412, var3 * 984.412, macroX * 684.412) / 512.0;
		var13 = var11 - var7;
		if (var13 < -10.0) {
			var13 = -10.0;
		}

		if (var13 > 10.0) {
			var13 = 10.0;
		}
	} else if (blockY > 1.0) {
		var11 = this->noiseGen2.GenerateOctaves(var1 * 684.412, var3 * 984.412, macroX * 684.412) / 512.0;
		var13 = var11 - var7;
		if (var13 < -10.0) {
			var13 = -10.0;
		}

		if (var13 > 10.0) {
			var13 = 10.0;
		}
	} else {
		double var15 =
			this->noiseGen1.GenerateOctaves(var1 * 684.412, var3 * 984.412, macroX * 684.412) / 512.0 - var7;
		double var17 =
			this->noiseGen2.GenerateOctaves(var1 * 684.412, var3 * 984.412, macroX * 684.412) / 512.0 - var7;
		if (var15 < -10.0) {
			var15 = -10.0;
		}

		if (var15 > 10.0) {
			var15 = 10.0;
		}

		if (var17 < -10.0) {
			var17 = -10.0;
		}

		if (var17 > 10.0) {
			var17 = 10.0;
		}

		double var19 = (blockY + 1.0) / 2.0;
		var11 = var15 + (var17 - var15) * var19;
		var13 = var11;
	}

	return var13;
}

bool GeneratorInfdev20100327::WorldGenMinableGenerate(BlockType blockType, World *pWorld, JavaRandom& pRand, int32_t var3,
													  int32_t var4, int32_t macroX) {
	float macroZ = pRand.nextFloat() * (float)JavaMath::PI;
	double var7 = double(float(var3 + 8) + MathHelper::sin(macroZ) * 2.0F);
	double blockY = double(float(var3 + 8) - MathHelper::sin(macroZ) * 2.0F);
	double world1 = double(float(macroX + 8) + MathHelper::cos(macroZ) * 2.0F);
	double world3 = double(float(macroX + 8) - MathHelper::cos(macroZ) * 2.0F);
	double world5 = double(var4 + pRand.nextInt(3) + 2);
	double world7 = double(var4 + pRand.nextInt(3) + 2);

	for (var3 = 0; var3 <= 16; ++var3) {
		double rand0 = var7 + (blockY - var7) * (double)var3 / 16.0;
		double rand2 = world5 + (world7 - world5) * (double)var3 / 16.0;
		double rand4 = world1 + (world3 - world1) * (double)var3 / 16.0;
		double rand6 = pRand.nextDouble();
		double rand8 = double(MathHelper::sin((float)var3 / 16.0F * (float)JavaMath::PI) + 1.0F) * rand6 + 1.0;
		double var30 = double(MathHelper::sin((float)var3 / 16.0F * (float)JavaMath::PI) + 1.0F) * rand6 + 1.0;

		for (var4 = Java::DoubleToInt32(rand0 - rand8 / 2.0); var4 <= Java::DoubleToInt32(rand0 + rand8 / 2.0); ++var4) {
			for (macroX = Java::DoubleToInt32(rand2 - var30 / 2.0); macroX <= Java::DoubleToInt32(rand2 + var30 / 2.0); ++macroX) {
				for (int32_t var41 = Java::DoubleToInt32(rand4 - rand8 / 2.0); var41 <= Java::DoubleToInt32(rand4 + rand8 / 2.0); ++var41) {
					double var35 = ((double)var4 + 0.5 - rand0) / (rand8 / 2.0);
					double var37 = ((double)macroX + 0.5 - rand2) / (var30 / 2.0);
					double var39 = ((double)var41 + 0.5 - rand4) / (rand8 / 2.0);
					if (var35 * var35 + var37 * var37 + var39 * var39 < 1.0 &&
						pWorld->GetBlockType(Int3{var4, macroX, var41}) == BLOCK_STONE) {
						pWorld->SetBlockTypeAndMeta(blockType, 0, Int3{var4, macroX, var41});
					}
				}
			}
		}
	}

	return true;
}

bool GeneratorInfdev20100327::PopulateChunk(Int2 chunkPos) {
	rand.setSeed(int64_t(chunkPos.x) * 318279123L + int64_t(chunkPos.y) * 919871212L);
	int32_t chunkZOffset = chunkPos.x << 4;
	chunkPos.x = chunkPos.y << 4;

	int32_t oreZ;
	int32_t oreY;
	int32_t oreX;
	for (chunkPos.y = 0; chunkPos.y < 20; ++chunkPos.y) {
		oreZ = chunkZOffset + rand.nextInt(16);
		oreY = rand.nextInt(128);
		oreX = chunkPos.x + rand.nextInt(16);
		WorldGenMinableGenerate(BLOCK_ORE_COAL, this->world, this->rand, oreZ, oreY, oreX);
	}

	for (chunkPos.y = 0; chunkPos.y < 10; ++chunkPos.y) {
		oreZ = chunkZOffset + rand.nextInt(16);
		oreY = rand.nextInt(64);
		oreX = chunkPos.x + rand.nextInt(16);
		WorldGenMinableGenerate(BLOCK_ORE_IRON, this->world, this->rand, oreZ, oreY, oreX);
	}

	if (rand.nextInt(2) == 0) {
		chunkPos.y = chunkZOffset + rand.nextInt(16);
		oreZ = rand.nextInt(32);
		oreY = chunkPos.x + rand.nextInt(16);
		WorldGenMinableGenerate(BLOCK_ORE_GOLD, this->world, this->rand, chunkPos.y, oreZ, oreY);
	}

	if (rand.nextInt(8) == 0) {
		chunkPos.y = chunkZOffset + rand.nextInt(16);
		oreZ = rand.nextInt(16);
		oreY = chunkPos.x + rand.nextInt(16);
		WorldGenMinableGenerate(BLOCK_ORE_DIAMOND, this->world, this->rand, chunkPos.y, oreZ, oreY);
	}

	chunkPos.y = Java::DoubleToInt32(this->mobSpawnerNoise.GenerateOctaves((double)chunkZOffset * 0.25, (double)chunkPos.x * 0.25)) << 3;

	for (oreZ = 0; oreZ < chunkPos.y; ++oreZ) {
		oreY = chunkZOffset + rand.nextInt(16);
		oreX = chunkPos.x + rand.nextInt(16);
		// Generate Trees
		int32_t heightValue = world->GetHeightValue(Int2{oreY, oreX});
		int32_t var7 = oreY + 2;
		int32_t blockY = oreX + 2;
		int32_t var10 = rand.nextInt(3) + 4;
		bool canPlaceTree = true;
		if (heightValue > 0 && heightValue + var10 + 1 <= CHUNK_HEIGHT) {
			int32_t treeY;
			int32_t var14;
			int32_t var15;
			int32_t blockId;
			for (treeY = heightValue; treeY <= heightValue + 1 + var10; ++treeY) {
				uint8_t var13 = 1;
				if (treeY == heightValue) {
					var13 = 0;
				}

				if (treeY >= heightValue + 1 + var10 - 2) {
					var13 = 2;
				}

				for (var14 = var7 - var13; var14 <= var7 + var13 && canPlaceTree; ++var14) {
					for (var15 = blockY - var13; var15 <= blockY + var13 && canPlaceTree; ++var15) {
						if (treeY >= 0 && treeY < CHUNK_HEIGHT) {
							if (world->GetBlockType(Int3{var14, treeY, var15}) != 0) {
								canPlaceTree = false;
							}
						} else {
							canPlaceTree = false;
						}
					}
				}
			}

			if (canPlaceTree) {
				treeY = world->GetBlockType(Int3{var7, heightValue - 1, blockY});
				if ((treeY == BLOCK_GRASS || treeY == BLOCK_DIRT) && heightValue < CHUNK_HEIGHT - var10 - 1) {
					world->SetBlockType(BLOCK_DIRT, Int3{var7, heightValue - 1, blockY});

					int32_t cX2;
					for (cX2 = heightValue - 3 + var10; cX2 <= heightValue + var10; ++cX2) {
						var14 = cX2 - (heightValue + var10);
						var15 = 1 - var14 / 2;

						for (blockId = var7 - var15; blockId <= var7 + var15; ++blockId) {
							int32_t cX1 = blockId - var7;

							for (treeY = blockY - var15; treeY <= blockY + var15; ++treeY) {
								int32_t var17 = treeY - blockY;
								if (((JavaMath::abs(cX1) != var15) || (JavaMath::abs(var17) != var15) ||
									 (rand.nextInt(2) != 0 && var14 != 0)) &&
									!IsOpaque(int16_t(world->GetBlockType(Int3{blockId, cX2, treeY})))) {
									world->SetBlockTypeAndMeta(BLOCK_LEAVES, 0, Int3{blockId, cX2, treeY});
								}
							}
						}
					}

					for (cX2 = 0; cX2 < var10; ++cX2) {
						if (!IsOpaque(int16_t(world->GetBlockType(Int3{var7, heightValue + cX2, blockY})))) {
							world->SetBlockTypeAndMeta(BLOCK_LOG, 0, Int3{var7, heightValue + cX2, blockY});
						}
					}
				}
			}
		}
	}
	return true;
}