#include "generatorBeta173.h"

GeneratorBeta173::GeneratorBeta173(int64_t seed, World* world) : Generator(seed, world) {
	logger = &Betrock::Logger::Instance();
    this->seed = seed;
    this->world = world;

    rand = std::make_unique<JavaRandom>(this->seed);
    noiseGen1 = std::make_unique<InfdevOctaves>(rand.get(), 16);
    noiseGen2 = std::make_unique<InfdevOctaves>(rand.get(), 16);
    noiseGen3 = std::make_unique<InfdevOctaves>(rand.get(), 8);
    noiseGen4 = std::make_unique<InfdevOctaves>(rand.get(), 4);
    noiseGen5 = std::make_unique<InfdevOctaves>(rand.get(), 4);
    noiseGen6 = std::make_unique<InfdevOctaves>(rand.get(), 10);
    noiseGen7 = std::make_unique<InfdevOctaves>(rand.get(), 16);
    mobSpawnerNoise = std::make_unique<InfdevOctaves>(rand.get(), 8);
}

std::unique_ptr<Chunk> GeneratorBeta173::GenerateChunk(int32_t cX, int32_t cZ) {
    std::unique_ptr<Chunk> c = std::make_unique<Chunk>(this->world,cX,cZ);
    this->rand->setSeed((long)cX * 341873128712L + (long)cZ * 132897987541L);
    std::memset(c->blocks, 0, sizeof(c->blocks));

	//this.biomesForGeneration = this.worldObj.getWorldChunkManager().loadBlockGeneratorData(this.biomesForGeneration, var1 * 16, var2 * 16, 16, 16);
    //double[] var5 = this.worldObj.getWorldChunkManager().temperature;
    std::vector<double> temperature;
    GenerateTerrain(cX, cZ, c, temperature);
    //this.replaceBlocksForBiome(var1, var2, var3, this.biomesForGeneration);
    //this.field_695_u.func_667_a(this, this.worldObj, var1, var2, var3);
    //var4.func_353_b();
    
    c->GenerateHeightMap();
    c->state = ChunkState::Generated;
    c->modified = true;
    return c;
}

bool GeneratorBeta173::PopulateChunk(int32_t cX, int32_t cZ) {
    return true;
}

void GeneratorBeta173::GenerateTerrain(int cX, int cY, std::unique_ptr<Chunk>& c, std::vector<double>& temperature) {//, BiomeGenBase[] var4, double[] var5) {
    uint8_t var6 = 4;
    uint8_t waterLevel = 64;
    int var8 = 4 + 1;
    uint8_t var9 = 17;
    int var10 = var6 + 1;
    
    this->field_4224_q = this->GenerateTerrainNoise(this->field_4224_q, cX * 4, 0, cY * 4, var8, var9, var10);

    for(int macroX = 0; macroX < 4; ++macroX) {
        for(int macroZ = 0; macroZ < 4; ++macroZ) {
            for(int macroY = 0; macroY < 16; ++macroY) {
                double eigthScaler = 0.125D;
                double var16 = this->field_4224_q[((macroX + 0) * var10 + macroZ + 0) * var9 + macroY + 0];
                double var18 = this->field_4224_q[((macroX + 0) * var10 + macroZ + 1) * var9 + macroY + 0];
                double var20 = this->field_4224_q[((macroX + 1) * var10 + macroZ + 0) * var9 + macroY + 0];
                double var22 = this->field_4224_q[((macroX + 1) * var10 + macroZ + 1) * var9 + macroY + 0];
                double var24 = (this->field_4224_q[((macroX + 0) * var10 + macroZ + 0) * var9 + macroY + 1] - var16) * eigthScaler;
                double var26 = (this->field_4224_q[((macroX + 0) * var10 + macroZ + 1) * var9 + macroY + 1] - var18) * eigthScaler;
                double var28 = (this->field_4224_q[((macroX + 1) * var10 + macroZ + 0) * var9 + macroY + 1] - var20) * eigthScaler;
                double var30 = (this->field_4224_q[((macroX + 1) * var10 + macroZ + 1) * var9 + macroY + 1] - var22) * eigthScaler;

                for(int subY = 0; subY < 8; ++subY) {
                    double quarterScale = 0.25D;
                    double var35 = var16;
                    double var37 = var18;
                    double var39 = (var20 - var16) * quarterScale;
                    double var41 = (var22 - var18) * quarterScale;

                    for(int subX = 0; subX < 4; ++subX) {
                        int blockIndex = subX + macroX * 4 << 11 | 0 + macroZ * 4 << 7 | macroY * 8 + subY;
                        short worldHeight = 128;
                        double var46 = 0.25D;
                        double var48 = var35;
                        double var50 = (var37 - var35) * var46;

                        for(int subZ = 0; subZ < 4; ++subZ) {
                            double var53 = 1.0; //temperature[(macroX * 4 + subX) * 16 + macroZ * 4 + subZ];
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

                    var16 += var24;
                    var18 += var26;
                    var20 += var28;
                    var22 += var30;
                }
            }
        }
    }
}

std::vector<double> GeneratorBeta173::GenerateTerrainNoise(std::vector<double> var1, int var2, int var3, int var4, int var5, int var6, int var7) {
    if(var1.empty()) {
        var1.reserve(var5 * var6 * var7);
        //var1 = new double[var5 * var6 * var7];
    }

    double var8 = 684.412D;
    double var10 = 684.412D;
    //double[] var12 = this->worldObj.getWorldChunkManager().temperature;
    //double[] var13 = this->worldObj.getWorldChunkManager().humidity;
    // This is some different noise generator
    this->field_4226_g = this->noiseGen6->func_4103_a(this->field_4226_g, var2, var4, var5, var7, 1.121D, 1.121D, 0.5D);
    this->field_4225_h = this->noiseGen7->func_4103_a(this->field_4225_h, var2, var4, var5, var7, 200.0D, 200.0D, 0.5D);
    this->field_4229_d = this->noiseGen3->generateNoiseOctaves(this->field_4229_d, (double)var2, (double)var3, (double)var4, var5, var6, var7, var8 / 80.0D, var10 / 160.0D, var8 / 80.0D);
    this->field_4228_e = this->noiseGen1->generateNoiseOctaves(this->field_4228_e, (double)var2, (double)var3, (double)var4, var5, var6, var7, var8, var10, var8);
    this->field_4227_f = this->noiseGen2->generateNoiseOctaves(this->field_4227_f, (double)var2, (double)var3, (double)var4, var5, var6, var7, var8, var10, var8);
    int var14 = 0;
    int var15 = 0;
    int var16 = 16 / var5;

    for(int var17 = 0; var17 < var5; ++var17) {
        int var18 = var17 * var16 + var16 / 2;

        for(int var19 = 0; var19 < var7; ++var19) {
            int var20 = var19 * var16 + var16 / 2;
            //double var21 = var12[var18 * 16 + var20];
            //double var23 = var13[var18 * 16 + var20] * var21;
            double var25 = 1.0D; // - var23;
            var25 *= var25;
            var25 *= var25;
            var25 = 1.0D - var25;
            double var27 = (this->field_4226_g[var15] + 256.0D) / 512.0D;
            var27 *= var25;
            if(var27 > 1.0D) {
                var27 = 1.0D;
            }

            double var29 = this->field_4225_h[var15] / 8000.0D;
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
            var29 = var29 * (double)var6 / 16.0D;
            double var31 = (double)var6 / 2.0D + var29 * 4.0D;
            ++var15;

            for(int var33 = 0; var33 < var6; ++var33) {
                double var34 = 0.0D;
                double var36 = ((double)var33 - var31) * 12.0D / var27;
                if(var36 < 0.0D) {
                    var36 *= 4.0D;
                }

                double var38 = this->field_4228_e[var14] / 512.0D;
                double var40 = this->field_4227_f[var14] / 512.0D;
                double var42 = (this->field_4229_d[var14] / 10.0D + 1.0D) / 2.0D;
                if(var42 < 0.0D) {
                    var34 = var38;
                } else if(var42 > 1.0D) {
                    var34 = var40;
                } else {
                    var34 = var38 + (var40 - var38) * var42;
                }

                var34 -= var36;
                if(var33 > var6 - 4) {
                    double var44 = (double)((float)(var33 - (var6 - 4)) / 3.0F);
                    var34 = var34 * (1.0D - var44) + -10.0D * var44;
                }

                var1[var14] = var34;
                ++var14;
            }
        }
    }

    return var1;
}