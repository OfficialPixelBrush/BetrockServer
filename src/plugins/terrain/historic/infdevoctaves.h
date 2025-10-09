#pragma once

#include "infdevperlin.h"
#include <vector>
#include <memory>

class InfdevOctaves {
    public:
        InfdevOctaves(int octaves);
        InfdevOctaves(JavaRandom* rand, int octaves);
        double noiseGenerator(double xOffset, double yOffset);
        double generateNoiseOctaves(double xOffset, double yOffset, double zOffset);
        std::vector<double> generateNoiseOctaves(std::vector<double> var1, double var2, double var4, double var6, int var8, int var9, int var10, double var11, double var13, double var15);
        std::vector<double> func_4103_a(std::vector<double> var1, int var2, int var3, int var4, int var5, double var6, double var8, double var10);
    private:
        int octaves;
        std::vector<std::unique_ptr<InfdevPerlin>> generatorCollection;
};