#include "infdevoctaves.h"

InfdevOctaves::InfdevOctaves(int octaves) : octaves(octaves) {
    JavaRandom rand;
    for (int i = 0; i < octaves; ++i) {
        generatorCollection.push_back(std::make_unique<InfdevPerlin>(rand));
    }
}

InfdevOctaves::InfdevOctaves(JavaRandom& rand, int octaves) {
    this->octaves = octaves;
    for(int i = 0; i < octaves; ++i) {
        generatorCollection.push_back(std::make_unique<InfdevPerlin>(rand));
    }
}

double InfdevOctaves::generateNoise(double xOffset, double yOffset) {
    double var5 = 0.0D;
    double var7 = 1.0D;

    for(int i = 0; i < this->octaves; ++i) {
        var5 += this->generatorCollection[i]->generateNoise(xOffset / var7, yOffset / var7) * var7;
        var7 *= 2.0D;
    }

    return var5;
}
double InfdevOctaves::generateNoise(double xOffset, double yOffset, double zOffset) {
    double var7 = 0.0D;
    double var9 = 1.0D;

    for(int i = 0; i < this->octaves; ++i) {
        var7 += this->generatorCollection[i]->generateNoiseD(xOffset / var9, 0.0D / var9, zOffset / var9) * var9;
        var9 *= 2.0D;
    }

    return var7;
}