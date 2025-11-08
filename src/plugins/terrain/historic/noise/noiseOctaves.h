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
        // Used by infdev
        double GenerateOctaves(double xOffset, double yOffset, double zOffset);
        // Used by Perlin
        double GenerateOctaves(double xOffset, double yOffset);
        std::vector<double> GenerateOctaves(std::vector<double>& noiseField, double var2, double var4, double var6, int var8, int var9, int var10, double var11, double var13, double var15);
        std::vector<double> GenerateOctaves(std::vector<double>& noiseField, int var2, int var3, int var4, int value, double var6, double var8, double var10);
        // Used by Simplex
        std::vector<double> GenerateOctaves(std::vector<double>& noiseField, double var2, double var4, int var6, int scale, double var8, double var10, double var12);
        std::vector<double> GenerateOctaves(std::vector<double>& noiseField, double var2, double var4, int var6, int scale, double var8, double var10, double var12, double var14);
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

// Only used by infdev
template <typename T>
double NoiseOctaves<T>::GenerateOctaves(double xOffset, double yOffset, double zOffset) {
    double value = 0.0D;
    double scale = 1.0D;

    for(int i = 0; i < this->octaves; ++i) {
        value += this->generatorCollection[i]->GenerateNoise(xOffset * scale, yOffset * scale, zOffset * scale) / scale;
        scale /= 2.0D;
    }

    return value;
}

// Used my Perlin
// func_647_a
template <typename T>
double NoiseOctaves<T>::GenerateOctaves(double xOffset, double yOffset) {
    double value = 0.0D;
    double scale = 1.0D;

    for(int i = 0; i < this->octaves; ++i) {
        value += this->generatorCollection[i]->GenerateNoise(xOffset * scale, yOffset * scale) / scale;
        scale /= 2.0D;
    }

    return value;
}

// generateNoiseOctaves
template <typename T>
std::vector<double> NoiseOctaves<T>::GenerateOctaves(std::vector<double>& noiseField, double var2, double var4, double var6, int var8, int var9, int var10, double var11, double var13, double var15) {
    if(noiseField.empty()) {
        noiseField.resize(var8 * var9 * var10, 0.0);
    } else {
        for(size_t i = 0; i < noiseField.size(); ++i) {
            noiseField[i] = 0.0D;
        }
    }

    double scale = 1.0D;

    for(int octave = 0; octave < this->octaves; ++octave) {
        this->generatorCollection[octave]->GenerateNoise(noiseField, var2, var4, var6, var8, var9, var10, var11 * scale, var13 * scale, var15 * scale, scale);
        scale /= 2.0D;
    }

    return noiseField;
}

// func_4103_a
template <typename T>
std::vector<double> NoiseOctaves<T>::GenerateOctaves(std::vector<double>& noiseField, int var2, int var3, int var4, int value, double var6, double var8, [[maybe_unused]] double var10) {
    return this->GenerateOctaves(noiseField, (double)var2, 10.0D, (double)var3, var4, 1, value, var6, 1.0D, var8);
}

// Comes from simplex Octaves
template <typename T>
std::vector<double> NoiseOctaves<T>::GenerateOctaves(std::vector<double>& noiseField, double var2, double var4, int var6, int scale, double var8, double var10, double var12) {
    return this->GenerateOctaves(noiseField, var2, var4, var6, scale, var8, var10, var12, 0.5D);
}

template <typename T>
std::vector<double> NoiseOctaves<T>::GenerateOctaves(std::vector<double>& noiseField, double var2, double var4, int var6, int scale, double var8, double var10, double var12, double var14) {
    var8 /= 1.5D;
    var10 /= 1.5D;
    if(!noiseField.empty() && int(noiseField.size()) >= var6 * scale) {
        for(size_t i = 0; i < noiseField.size(); ++i) {
            noiseField[i] = 0.0D;
        }
    } else {
        noiseField.resize(var6 * scale, 0.0);
    }

    double var21 = 1.0D;
    double var18 = 1.0D;

    for(int octave = 0; octave < this->octaves; ++octave) {
        this->generatorCollection[octave]->GenerateNoise(noiseField, var2, var4, var6, scale, var8 * var18, var10 * var18, 0.55D / var21);
        var18 *= var12;
        var21 *= var14;
    }

    return noiseField;
}