// A recreation of the the Infdev 20100227-1433 Perlin noise function
#pragma once
#include "noiseGenerator.h"

class NoisePerlin : public NoiseGenerator {
    private:
        int permutations[512];
        double xCoord;
        double yCoord;
        double zCoord;
        double GenerateNoiseBase(double x, double y, double z);
    public:
        NoisePerlin();
        NoisePerlin(JavaRandom* rand);
        double GenerateNoise(double x, double y);
        double GenerateNoise(double x, double y, double z);
        void GenerateNoise(std::vector<double>& var1, double var2, double var4, double var6, int var8, int var9, int var10, double var11, double var13, double var15, double var17);
};