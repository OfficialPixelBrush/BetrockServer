// A recreation of the the Infdev 20100227-1433 Perlin noise function
#pragma once

#include "javaRandom.h"

class InfdevPerlin {
    private:
        int permutations[512];
        double xCoord;
        double yCoord;
        double zCoord;
        double generateNoise(double x, double y, double z);
    public:
        InfdevPerlin();
        InfdevPerlin(JavaRandom* rand);
        double generateNoise(double x, double y);
        double generateNoiseD(double x, double y, double z);
        void func_646_a(std::vector<double>& var1, double var2, double var4, double var6, int var8, int var9, int var10, double var11, double var13, double var15, double var17);
};

// Linear Interpolation
static double lerp(double a, double b, double t) {
    return b + a * (t - b);
}

static double grad(int var0, double var1, double var3, double var5) {
    var0 &= 15;
    double var8 = var0 < 8 ? var1 : var3;
    double var10 = var0 < 4 ? var3 : (var0 != 12 && var0 != 14 ? var5 : var1);
    return ((var0 & 1) == 0 ? var8 : -var8) + ((var0 & 2) == 0 ? var10 : -var10);
}

static double altGrad(int var1, double var2, double var4) {
    int var6 = var1 & 15;
    double var7 = (double)(1 - ((var6 & 8) >> 3)) * var2;
    double var9 = var6 < 4 ? 0.0D : (var6 != 12 && var6 != 14 ? var4 : var2);
    return ((var6 & 1) == 0 ? var7 : -var7) + ((var6 & 2) == 0 ? var9 : -var9);
}

// Easing Function
static double fade(double value) {
    return value * value * value * (value * (value * 6.0D - 15.0D) + 10.0D);
}