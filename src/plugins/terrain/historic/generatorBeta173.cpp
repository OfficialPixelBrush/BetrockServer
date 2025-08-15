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
    c->GenerateHeightMap();
    c->state = ChunkState::Generated;
    c->modified = true;
    return c;
}

// TODO: Implement this
bool GeneratorBeta173::PopulateChunk(int32_t cX, int32_t cZ) {
    return true;
}

/*
public void generateTerrain(int var1, int var2, int8_t[] var3, BiomeGenBase[] var4, double[] var5) {
    int8_t var6 = 4;
    int8_t var7 = 64;
    int var8 = var6 + 1;
    int8_t var9 = 17;
    int var10 = var6 + 1;
    //this.field_4180_q = this.func_4061_a(this.field_4180_q, var1 * var6, 0, var2 * var6, var8, var9, var10);

    for(int var11 = 0; var11 < var6; ++var11) {
        for(int var12 = 0; var12 < var6; ++var12) {
            for(int var13 = 0; var13 < 16; ++var13) {
                double var14 = 0.125D;
                double var16 = this.field_4180_q[((var11 + 0) * var10 + var12 + 0) * var9 + var13 + 0];
                double var18 = this.field_4180_q[((var11 + 0) * var10 + var12 + 1) * var9 + var13 + 0];
                double var20 = this.field_4180_q[((var11 + 1) * var10 + var12 + 0) * var9 + var13 + 0];
                double var22 = this.field_4180_q[((var11 + 1) * var10 + var12 + 1) * var9 + var13 + 0];
                double var24 = (this.field_4180_q[((var11 + 0) * var10 + var12 + 0) * var9 + var13 + 1] - var16) * var14;
                double var26 = (this.field_4180_q[((var11 + 0) * var10 + var12 + 1) * var9 + var13 + 1] - var18) * var14;
                double var28 = (this.field_4180_q[((var11 + 1) * var10 + var12 + 0) * var9 + var13 + 1] - var20) * var14;
                double var30 = (this.field_4180_q[((var11 + 1) * var10 + var12 + 1) * var9 + var13 + 1] - var22) * var14;

                for(int var32 = 0; var32 < 8; ++var32) {
                    double var33 = 0.25D;
                    double var35 = var16;
                    double var37 = var18;
                    double var39 = (var20 - var16) * var33;
                    double var41 = (var22 - var18) * var33;

                    for(int var43 = 0; var43 < 4; ++var43) {
                        int var44 = var43 + var11 * 4 << 11 | 0 + var12 * 4 << 7 | var13 * 8 + var32;
                        short var45 = 128;
                        double var46 = 0.25D;
                        double var48 = var35;
                        double var50 = (var37 - var35) * var46;

                        for(int var52 = 0; var52 < 4; ++var52) {
                            double var53 = var5[(var11 * 4 + var43) * 16 + var12 * 4 + var52];
                            int var55 = 0;
                            if(var13 * 8 + var32 < var7) {
                                if(var53 < 0.5D && var13 * 8 + var32 >= var7 - 1) {
                                    var55 = BLOCK_ICE;
                                } else {
                                    var55 = BLOCK_WATER_STILL;
                                }
                            }

                            if(var48 > 0.0D) {
                                var55 = BLOCK_STONE;
                            }

                            var3[var44] = (int8_t)var55;
                            var44 += var45;
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
*/