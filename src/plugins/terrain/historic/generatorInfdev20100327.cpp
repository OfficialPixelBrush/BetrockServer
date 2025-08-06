#include "generatorInfdev20100327.h"

GeneratorInfdev20100327::GeneratorInfdev20100327() {
	logger = &Betrock::Logger::Instance();
    this->seed = seed;
    this->world = world;

    rand = JavaRandom(this->seed);
    noiseGen1 = std::make_unique<InfdevOctaves>(rand, 16);
    noiseGen2 = std::make_unique<InfdevOctaves>(rand, 16);
    noiseGen3 = std::make_unique<InfdevOctaves>(rand, 8);
    noiseGen4 = std::make_unique<InfdevOctaves>(rand, 4);
    noiseGen5 = std::make_unique<InfdevOctaves>(rand, 4);
    noiseGen6 = std::make_unique<InfdevOctaves>(rand, 5);
    mobSpawnerNoise = std::make_unique<InfdevOctaves>(rand, 5);
}

Chunk GeneratorInfdev20100327::GenerateChunk(int32_t cX, int32_t cZ) {
    Chunk c = Chunk();
    int var3 = cX << 4;
    int var14 = cZ << 4;
    int var4 = 0;

    for(int var5 = var3; var5 < var3 + 16; ++var5) {
        for(int var6 = var14; var6 < var14 + 16; ++var6) {
            int var7 = var5 / 1024;
            int var8 = var6 / 1024;
            float var9 = (float)(this->noiseGen1.get()->generateNoise((double)((float)var5 / 0.03125F), 0.0D, (double)((float)var6 / 0.03125F)) - this->noiseGen2.get()->generateNoise((double)((float)var5 / 0.015625F), 0.0D, (double)((float)var6 / 0.015625F))) / 512.0F / 4.0F;
            float var10 = (float)this->noiseGen5.get()->generateNoise((double)((float)var5 / 4.0F), (double)((float)var6 / 4.0F));
            float var11 = (float)this->noiseGen6.get()->generateNoise((double)((float)var5 / 8.0F), (double)((float)var6 / 8.0F)) / 8.0F;
            var10 = var10 > 0.0F ? (float)(this->noiseGen3.get()->generateNoise((double)((float)var5 * 0.25714284F * 2.0F), (double)((float)var6 * 0.25714284F * 2.0F)) * (double)var11 / 4.0D) : (float)(this->noiseGen4.get()->generateNoise((double)((float)var5 * 0.25714284F), (double)((float)var6 * 0.25714284F)) * (double)var11);
            int var15 = (int)(var9 + 64.0F + var10);
            if((float)this->noiseGen5.get()->generateNoise((double)var5, (double)var6) < 0.0F) {
                var15 = var15 / 2 << 1;
                if((float)this->noiseGen5.get()->generateNoise((double)(var5 / 5), (double)(var6 / 5)) < 0.0F) {
                    ++var15;
                }
            }

            float value = static_cast<float>(std::rand()) / RAND_MAX;

            for(int var16 = 0; var16 < 128; ++var16) {
                int var17 = 0;
                if((var5 == 0 || var6 == 0) && var16 <= var15 + 2) {
                    var17 = BLOCK_OBSIDIAN;
                } else if(var16 == var15 + 1 && var15 >= 64 && value < 0.02D) {
                    var17 = BLOCK_DANDELION;
                } else if(var16 == var15 && var15 >= 64) {
                    var17 = BLOCK_GRASS;
                } else if(var16 <= var15 - 2) {
                    var17 = BLOCK_STONE;
                } else if(var16 <= var15) {
                    var17 = BLOCK_DIRT;
                } else if(var16 <= 64) {
                    var17 = BLOCK_WATER_STILL;
                }

                this->rand.setSeed((long)(var7 + var8 * 13871));
                int var12 = (var7 << 10) + 128 + this->rand.nextInt(512);
                int var13 = (var8 << 10) + 128 + this->rand.nextInt(512);
                var12 = var5 - var12;
                var13 = var6 - var13;
                if(var12 < 0) {
                    var12 = -var12;
                }

                if(var13 < 0) {
                    var13 = -var13;
                }

                if(var13 > var12) {
                    var12 = var13;
                }

                var12 = 127 - var12;
                if(var12 == 255) {
                    var12 = 1;
                }

                if(var12 < var15) {
                    var12 = var15;
                }

                if(var16 <= var12 && (var17 == 0 || var17 == BLOCK_WATER_STILL)) {
                    var17 = BLOCK_BRICKS;
                }

                if(var17 < 0) {
                    var17 = 0;
                }

                Block b;
                b.type = var17;
                c.blocks[var4++] = b;
            }
        }
    }
    // To prevent population
    c.populated = true;
    CalculateChunkLight(&c);
    c.modified = true;
    return c;
}
