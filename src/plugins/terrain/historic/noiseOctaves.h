#pragma once

#include "noiseGenerator.h"
#include "noisePerlin.h"
#include "noiseSimplex.h"
#include <vector>
#include <memory>

template <typename T>
class NoiseOctaves {
    public:
        NoiseOctaves(int octaves);
        NoiseOctaves(JavaRandom* rand, int octaves);
        double GenerateOctaves(double xOffset, double yOffset);
        double GenerateOctaves(double xOffset, double yOffset, double zOffset);
        std::vector<double> GenerateOctaves(std::vector<double> var1, double var2, double var4, double var6, int var8, int var9, int var10, double var11, double var13, double var15);
        std::vector<double> GenerateOctaves(std::vector<double> var1, int var2, int var3, int var4, int var5, double var6, double var8, double var10);
        std::vector<double> GenerateOctaves(std::vector<double> var1, double var2, double var4, int var6, int var7, double var8, double var10, double var12);
        std::vector<double> GenerateOctaves(std::vector<double> var1, double var2, double var4, int var6, int var7, double var8, double var10, double var12, double var14);
    private:
        int octaves;
        std::vector<std::unique_ptr<T>> generatorCollection;
};


template <typename T>
NoiseOctaves<T>::NoiseOctaves(int octaves) : octaves(octaves) {
    JavaRandom* rand = new JavaRandom();
    for (int i = 0; i < octaves; ++i) {
        generatorCollection.push_back(std::make_unique<T>(rand));
    }
}

template <typename T>
NoiseOctaves<T>::NoiseOctaves(JavaRandom* rand, int octaves) {
    this->octaves = octaves;
    for(int i = 0; i < octaves; ++i) {
        generatorCollection.push_back(std::make_unique<T>(rand));
    }
}

template <typename T>
double NoiseOctaves<T>::GenerateOctaves(double xOffset, double yOffset) {
    double var5 = 0.0D;
    double var7 = 1.0D;

    for(int i = 0; i < this->octaves; ++i) {
        var5 += this->generatorCollection[i]->GenerateNoise(xOffset / var7, yOffset / var7) * var7;
        var7 *= 2.0D;
    }

    return var5;
}

template <typename T>
double NoiseOctaves<T>::GenerateOctaves(double xOffset, double yOffset, double zOffset) {
    double var7 = 0.0D;
    double var9 = 1.0D;

    for(int i = 0; i < this->octaves; ++i) {
        var7 += this->generatorCollection[i]->GenerateNoise(xOffset / var9, yOffset / var9, zOffset / var9) * var9;
        var9 *= 2.0D;
    }

    return var7;
}

template <typename T>
std::vector<double> NoiseOctaves<T>::GenerateOctaves(std::vector<double> var1, double var2, double var4, double var6, int var8, int var9, int var10, double var11, double var13, double var15) {
    if(var1.empty()) {
        var1.resize(var8 * var9 * var10, 0.0);
    } else {
        for(int var17 = 0; var17 < var1.size(); ++var17) {
            var1[var17] = 0.0D;
        }
    }

    double var20 = 1.0D;

    for(int var19 = 0; var19 < this->octaves; ++var19) {
        this->generatorCollection[var19]->GenerateNoise(var1, var2, var4, var6, var8, var9, var10, var11 * var20, var13 * var20, var15 * var20, var20);
        var20 /= 2.0D;
    }

    return var1;
}

template <typename T>
std::vector<double> NoiseOctaves<T>::GenerateOctaves(std::vector<double> var1, int var2, int var3, int var4, int var5, double var6, double var8, double var10) {
    return this->GenerateOctaves(var1, (double)var2, 10.0D, (double)var3, var4, 1, var5, var6, 1.0D, var8);
}

template <typename T>
std::vector<double> NoiseOctaves<T>::GenerateOctaves(std::vector<double> var1, double var2, double var4, int var6, int var7, double var8, double var10, double var12) {
    return this->GenerateOctaves(var1, var2, var4, var6, var7, var8, var10, var12, 0.5D);
}

template <typename T>
std::vector<double> NoiseOctaves<T>::GenerateOctaves(std::vector<double> var1, double var2, double var4, int var6, int var7, double var8, double var10, double var12, double var14) {
    var8 /= 1.5D;
    var10 /= 1.5D;
    if(!var1.empty() && var1.size() >= var6 * var7) {
        for(int var16 = 0; var16 < var1.size(); ++var16) {
            var1[var16] = 0.0D;
        }
    } else {
        var1.resize(var6 * var7, 0.0);
    }

    double var21 = 1.0D;
    double var18 = 1.0D;

    for(int var20 = 0; var20 < this->octaves; ++var20) {
        this->generatorCollection[var20]->GenerateNoise(var1, var2, var4, var6, var7, var8 * var18, var10 * var18, 0.55D / var21);
        var18 *= var12;
        var21 *= var14;
    }

    return var1;
}