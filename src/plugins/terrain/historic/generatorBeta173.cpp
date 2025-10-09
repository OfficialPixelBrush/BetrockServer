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
    GenerateTerrain(cX,cZ,c);
    return c;
}

bool GeneratorBeta173::PopulateChunk(int32_t cX, int32_t cZ) {
    return true;
}

void GeneratorBeta173::GenerateTerrain(int cX, int cY, std::unique_ptr<Chunk> c) {//, BiomeGenBase[] var4, double[] var5) {
    uint8_t var6 = 4;
    uint8_t waterLevel = 64;
    int var8 = 4 + 1;
    uint8_t var9 = 17;
    int var10 = var6 + 1;
    this.field_4224_q = this.func_4058_a(this.field_4224_q, cX * 4, 0, cY * 4, var8, var9, var10);

    for(int macroX = 0; macroX < 4; ++macroX) {
        for(int macroZ = 0; macroZ < 4; ++macroZ) {
            for(int macroY = 0; macroY < 16; ++macroY) {
                double eigthScaler = 0.125D;
                double var16 = this.field_4224_q[((macroX + 0) * var10 + macroZ + 0) * var9 + macroY + 0];
                double var18 = this.field_4224_q[((macroX + 0) * var10 + macroZ + 1) * var9 + macroY + 0];
                double var20 = this.field_4224_q[((macroX + 1) * var10 + macroZ + 0) * var9 + macroY + 0];
                double var22 = this.field_4224_q[((macroX + 1) * var10 + macroZ + 1) * var9 + macroY + 0];
                double var24 = (this.field_4224_q[((macroX + 0) * var10 + macroZ + 0) * var9 + macroY + 1] - var16) * eigthScaler;
                double var26 = (this.field_4224_q[((macroX + 0) * var10 + macroZ + 1) * var9 + macroY + 1] - var18) * eigthScaler;
                double var28 = (this.field_4224_q[((macroX + 1) * var10 + macroZ + 0) * var9 + macroY + 1] - var20) * eigthScaler;
                double var30 = (this.field_4224_q[((macroX + 1) * var10 + macroZ + 1) * var9 + macroY + 1] - var22) * eigthScaler;

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
                            double var53 = var5[(macroX * 4 + subX) * 16 + macroZ * 4 + subZ];
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
                            c.blocks[blockIndex] = (uint8_t)blockType;
                            
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