#include "generatorInfdev20100327.h"

GeneratorInfdev20100327::GeneratorInfdev20100327(int64_t seed, World* world) : Generator(seed, world) {
	logger = &Betrock::Logger::Instance();
    this->seed = seed;
    this->world = world;

    rand = std::make_unique<JavaRandom>(this->seed);
    noiseGen1 = std::make_unique<InfdevOctaves>(rand.get(), 16);
    noiseGen2 = std::make_unique<InfdevOctaves>(rand.get(), 16);
    noiseGen3 = std::make_unique<InfdevOctaves>(rand.get(), 8);
    noiseGen4 = std::make_unique<InfdevOctaves>(rand.get(), 4);
    noiseGen5 = std::make_unique<InfdevOctaves>(rand.get(), 4);
    noiseGen6 = std::make_unique<InfdevOctaves>(rand.get(), 5);
    mobSpawnerNoise = std::make_unique<InfdevOctaves>(rand.get(), 5);
}

std::unique_ptr<Chunk> GeneratorInfdev20100327::GenerateChunk(int32_t cX, int32_t cZ) {
    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(this->world,cX,cZ);
    this->rand->setSeed((long)cX * 341873128712L + (long)cZ * 132897987541L);
    std::memset(c->blocks, 0, sizeof(c->blocks));

    int var5;
    int var6;
    int var8;
    int var9;
    for(var5 = 0; var5 < 4; ++var5) {
        for(var6 = 0; var6 < 4; ++var6) {
            double var7[33][4];
            var8 = (cX << 2) + var5;
            var9 = (cZ << 2) + var6;

            // 33 == size of var7's first
            for(int var10 = 0; var10 < 33; ++var10) {
                var7[var10][0] = this->InitializeNoiseField((double)var8, (double)var10, (double)var9);
                var7[var10][1] = this->InitializeNoiseField((double)var8, (double)var10, (double)(var9 + 1));
                var7[var10][2] = this->InitializeNoiseField((double)(var8 + 1), (double)var10, (double)var9);
                var7[var10][3] = this->InitializeNoiseField((double)(var8 + 1), (double)var10, (double)(var9 + 1));
            }

            for(var8 = 0; var8 < 32; ++var8) {
                double var50 = var7[var8][0];
                double var11 = var7[var8][1];
                double var13 = var7[var8][2];
                double var15 = var7[var8][3];
                double var17 = var7[var8 + 1][0];
                double var19 = var7[var8 + 1][1];
                double var21 = var7[var8 + 1][2];
                double var23 = var7[var8 + 1][3];

                for(int var25 = 0; var25 < 4; ++var25) {
                    double var26 = (double)var25 / 4.0D;
                    double var28 = var50 + (var17 - var50) * var26;
                    double var30 = var11 + (var19 - var11) * var26;
                    double var32 = var13 + (var21 - var13) * var26;
                    double var34 = var15 + (var23 - var15) * var26;
                    for(int var51 = 0; var51 < 4; ++var51) {
                        double var37 = (double)var51 / 4.0D;
                        double var39 = var28 + (var32 - var28) * var37;
                        double var41 = var30 + (var34 - var30) * var37;
                        int var27 = var51 + (var5 << 2) << 11 | 0 + (var6 << 2) << 7 | (var8 << 2) + var25;

                        for(int var36 = 0; var36 < 4; ++var36) {
                            double var45 = (double)var36 / 4.0D;
                            double var47 = var39 + (var41 - var39) * var45;
                            int var52 = 0;
                            if((var8 << 2) + var25 < 64) {
                                var52 = BLOCK_WATER_STILL;
                            }

                            if(var47 > 0.0D) {
                                var52 = BLOCK_STONE;
                            }

                            c->blocks[var27].type = (char)var52;
                            var27 += 128;
                        }
                    }
                }
            }
        }
    }

    for(var5 = 0; var5 < 16; ++var5) {
        for(var6 = 0; var6 < 16; ++var6) {
            int var49 = var5 << 11 | var6 << 7 | 127;
            var8 = -1;

            for(var9 = 127; var9 >= 0; --var9) {
                if(c->blocks[var49].type == BLOCK_AIR) {
                    var8 = -1;
                } else if(c->blocks[var49].type == BLOCK_STONE) {
                    if(var8 == -1) {
                        var8 = 3;
                        if(var9 >= 63) {
                            c->blocks[var49].type = BLOCK_GRASS;
                        } else {
                            c->blocks[var49].type = BLOCK_DIRT;
                        }
                    } else if(var8 > 0) {
                        --var8;
                        c->blocks[var49].type = BLOCK_DIRT;
                    }
                }

                --var49;
            }
        }
    }
    
    c->GenerateHeightMap();
    c->state = ChunkState::Generated;
    c->modified = true;
    return c;
}

double GeneratorInfdev20100327::InitializeNoiseField(double var1, double var3, double var5) {
    double var7 = var3 * 4.0D - 64.0D;
    if(var7 < 0.0D) {
        var7 *= 3.0D;
    }

    double var9 = this->noiseGen3->generateNoiseOctaves(var1 * 684.412D / 80.0D, var3 * 684.412D / 400.0D, var5 * 684.412D / 80.0D) / 2.0D;
    double var11;
    double var13;
    if(var9 < -1.0D) {
        var11 = this->noiseGen1->generateNoiseOctaves(var1 * 684.412D, var3 * 984.412D, var5 * 684.412D) / 512.0D;
        var13 = var11 - var7;
        if(var13 < -10.0D) {
            var13 = -10.0D;
        }

        if(var13 > 10.0D) {
            var13 = 10.0D;
        }
    } else if(var9 > 1.0D) {
        var11 = this->noiseGen2->generateNoiseOctaves(var1 * 684.412D, var3 * 984.412D, var5 * 684.412D) / 512.0D;
        var13 = var11 - var7;
        if(var13 < -10.0D) {
            var13 = -10.0D;
        }

        if(var13 > 10.0D) {
            var13 = 10.0D;
        }
    } else {
        double var15 = this->noiseGen1->generateNoiseOctaves(var1 * 684.412D, var3 * 984.412D, var5 * 684.412D) / 512.0D - var7;
        double var17 = this->noiseGen2->generateNoiseOctaves(var1 * 684.412D, var3 * 984.412D, var5 * 684.412D) / 512.0D - var7;
        if(var15 < -10.0D) {
            var15 = -10.0D;
        }

        if(var15 > 10.0D) {
            var15 = 10.0D;
        }

        if(var17 < -10.0D) {
            var17 = -10.0D;
        }

        if(var17 > 10.0D) {
            var17 = 10.0D;
        }

        double var19 = (var9 + 1.0D) / 2.0D;
        var11 = var15 + (var17 - var15) * var19;
        var13 = var11;
    }

    return var13;
}

bool WorldGenMinableGenerate(int blockType, World* world, JavaRandom* rand, int var3, int var4, int var5) {
    float var6 = rand->nextFloat() * (float)M_PI;
    double var7 = (double)((float)(var3 + 8) + std::sin(var6) * 2.0F);
    double var9 = (double)((float)(var3 + 8) - std::sin(var6) * 2.0F);
    double world1 = (double)((float)(var5 + 8) + std::cos(var6) * 2.0F);
    double world3 = (double)((float)(var5 + 8) - std::cos(var6) * 2.0F);
    double world5 = (double)(var4 + rand->nextInt(3) + 2);
    double world7 = (double)(var4 + rand->nextInt(3) + 2);

    for(var3 = 0; var3 <= 16; ++var3) {
        double rand0 = var7 + (var9 - var7) * (double)var3 / 16.0D;
        double rand2 = world5 + (world7 - world5) * (double)var3 / 16.0D;
        double rand4 = world1 + (world3 - world1) * (double)var3 / 16.0D;
        double rand6 = rand->nextDouble();
        double rand8 = (double)(std::sin((float)var3 / 16.0F * (float)M_PI) + 1.0F) * rand6 + 1.0D;
        double var30 = (double)(std::sin((float)var3 / 16.0F * (float)M_PI) + 1.0F) * rand6 + 1.0D;

        for(var4 = (int)(rand0 - rand8 / 2.0D); var4 <= (int)(rand0 + rand8 / 2.0D); ++var4) {
            for(var5 = (int)(rand2 - var30 / 2.0D); var5 <= (int)(rand2 + var30 / 2.0D); ++var5) {
                for(int var41 = (int)(rand4 - rand8 / 2.0D); var41 <= (int)(rand4 + rand8 / 2.0D); ++var41) {
                    double var35 = ((double)var4 + 0.5D - rand0) / (rand8 / 2.0D);
                    double var37 = ((double)var5 + 0.5D - rand2) / (var30 / 2.0D);
                    double var39 = ((double)var41 + 0.5D - rand4) / (rand8 / 2.0D);
                    Block* b = world->GetBlock(Int3{var4, var5, var41});
                    if (!b) continue;
                    if(var35 * var35 + var37 * var37 + var39 * var39 < 1.0D && b->type == BLOCK_STONE) {
                        b->type = blockType;
                        b->meta = 0;
                    }
                }
            }
        }
    }

    return true;
}

bool GeneratorInfdev20100327::PopulateChunk(int32_t cX, int32_t cZ) {
	this->rand->setSeed((int64_t)cX * 318279123L + (int64_t)cZ * 919871212L);
	int chunkZOffset = cX << 4;
	cX = cZ << 4;

	int oreZ;
	int oreY;
	int oreX;
	for(cZ = 0; cZ < 20; ++cZ) {
		oreZ = chunkZOffset + this->rand->nextInt(16);
		oreY = this->rand->nextInt(128);
		oreX = cX + this->rand->nextInt(16);
		WorldGenMinableGenerate(BLOCK_ORE_COAL, this->world, this->rand.get(), oreZ, oreY, oreX);
	}

	for(cZ = 0; cZ < 10; ++cZ) {
		oreZ = chunkZOffset + this->rand->nextInt(16);
		oreY = this->rand->nextInt(64);
		oreX = cX + this->rand->nextInt(16);
		WorldGenMinableGenerate(BLOCK_ORE_IRON, this->world, this->rand.get(), oreZ, oreY, oreX);
	}

	if(this->rand->nextInt(2) == 0) {
		cZ = chunkZOffset + this->rand->nextInt(16);
		oreZ = this->rand->nextInt(32);
		oreY = cX + this->rand->nextInt(16);
		WorldGenMinableGenerate(BLOCK_ORE_GOLD, this->world, this->rand.get(), cZ, oreZ, oreY);
	}

	if(this->rand->nextInt(8) == 0) {
		cZ = chunkZOffset + this->rand->nextInt(16);
		oreZ = this->rand->nextInt(16);
		oreY = cX + this->rand->nextInt(16);
		WorldGenMinableGenerate(BLOCK_ORE_DIAMOND, this->world, this->rand.get(), cZ, oreZ, oreY);
	}

	cZ = (int)this->mobSpawnerNoise->noiseGenerator((double)chunkZOffset * 0.25D, (double)cX * 0.25D) << 3;

	for(oreZ = 0; oreZ < cZ; ++oreZ) {
		oreY = chunkZOffset + this->rand->nextInt(16);
		oreX = cX + this->rand->nextInt(16);
		//new WorldGenTrees();
		int heightValue = this->world->GetHeightValue(oreY, oreX);
		int var7 = oreY + 2;
		int var9 = oreX + 2;
		int var10 = this->rand->nextInt(3) + 4;
		bool canPlaceTree = true;
		bool cX3;
		if(heightValue > 0 && heightValue + var10 + 1 <= 128) {
			int treeY;
			int var14;
			int var15;
			int blockId;
			for(treeY = heightValue; treeY <= heightValue + 1 + var10; ++treeY) {
				uint8_t var13 = 1;
				if(treeY == heightValue) {
					var13 = 0;
				}

				if(treeY >= heightValue + 1 + var10 - 2) {
					var13 = 2;
				}

				for(var14 = var7 - var13; var14 <= var7 + var13 && canPlaceTree; ++var14) {
					for(var15 = var9 - var13; var15 <= var9 + var13 && canPlaceTree; ++var15) {
						if(treeY >= 0 && treeY < 128) {
							if(world->GetBlockType(Int3{var14, treeY, var15}) != 0) {
								canPlaceTree = false;
							}
						} else {
							canPlaceTree = false;
						}
					}
				}
			}

			if(!canPlaceTree) {
				cX3 = false;
			} else {
                Block* b = world->GetBlock(Int3{var7, heightValue - 1, var9});
                if (!b) break;
				treeY = b->type;
				if((treeY == BLOCK_GRASS || treeY == BLOCK_DIRT) && heightValue < 128 - var10 - 1) {
				    b->type = BLOCK_DIRT;

					int cX2;
					for(cX2 = heightValue - 3 + var10; cX2 <= heightValue + var10; ++cX2) {
						var14 = cX2 - (heightValue + var10);
						var15 = 1 - var14 / 2;

						for(blockId = var7 - var15; blockId <= var7 + var15; ++blockId) {
							int cX1 = blockId - var7;

							for(treeY = var9 - var15; treeY <= var9 + var15; ++treeY) {
								int var17 = treeY - var9;
                                Block* b = world->GetBlock(Int3{blockId, cX2, treeY});
                                if (!b) continue;
								if((std::abs(cX1) != var15 || std::abs(var17) != var15 || this->rand->nextInt(2) != 0 && var14 != 0) && !IsOpaque(b->type)) {
                                    b->type = BLOCK_LEAVES;
                                    b->meta = 0;
								}
							}
						}
					}

					for(cX2 = 0; cX2 < var10; ++cX2) {
                        Block* b = world->GetBlock(Int3{var7, heightValue + cX2, var9});
                        if (!b) continue;
						if(!IsOpaque(b->type)) {
                            b->type = BLOCK_LOG;
                            b->meta = 0;
						}
					}

					cX3 = true;
				} else {
					cX3 = false;
				}
			}
		} else {
			cX3 = false;
		}
	}
    return true;
}