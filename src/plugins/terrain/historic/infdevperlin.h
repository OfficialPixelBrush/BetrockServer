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

// Easing Function
static double fade(double value) {
    return value * value * value * (value * (value * 6.0D - 15.0D) + 10.0D);
}