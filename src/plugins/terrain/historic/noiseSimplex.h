// A recreation of the the Infdev 20100227-1433 Perlin noise function
#pragma once
#include "noiseGenerator.h"

class NoiseSimplex : public NoiseGenerator {
    private:
        int permutations[512];
        double xCoord;
        double yCoord;
        double zCoord;
        double GenerateNoiseBase(double x, double y, double z);
        int gradients[12][3] = {
            {1, 1, 0},
            {-1, 1, 0},
            {1, -1, 0},
            {-1, -1, 0},
            {1, 0, 1},
            {-1, 0, 1},
            {1, 0, -1},
            {-1, 0, -1},
            {0, 1, 1},
            {0, -1, 1},
            {0, 1, -1},
            {0, -1, -1}
        };
        double field_4315_f = 0.5D * (sqrt(3.0D) - 1.0D);
        double field_4314_g = (3.0D - sqrt(3.0D)) / 6.0D;
    public:
        NoiseSimplex();
        NoiseSimplex(JavaRandom* rand);
        void GenerateNoise(std::vector<double> var1, double var2, double var4, int var6, int var7, double var8, double var10, double var12);
};

static int wrap(double var0) {
    return var0 > 0.0D ? (int)var0 : (int)var0 - 1;
}

static double func_4114_a(int var0[3], double var1, double var3) {
    return (double)var0[0] * var1 + (double)var0[1] * var3;
}