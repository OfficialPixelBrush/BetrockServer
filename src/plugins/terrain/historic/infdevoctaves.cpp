#include "infdevoctaves.h"

InfdevOctaves::InfdevOctaves(int octaves) : octaves(octaves) {
    JavaRandom* rand = new JavaRandom();
    for (int i = 0; i < octaves; ++i) {
        generatorCollection.push_back(std::make_unique<InfdevPerlin>(rand));
    }
}

InfdevOctaves::InfdevOctaves(JavaRandom* rand, int octaves) {
    this->octaves = octaves;
    for(int i = 0; i < octaves; ++i) {
        generatorCollection.push_back(std::make_unique<InfdevPerlin>(rand));
    }
}

double InfdevOctaves::noiseGenerator(double xOffset, double yOffset) {
    double var5 = 0.0D;
    double var7 = 1.0D;

    for(int i = 0; i < this->octaves; ++i) {
        var5 += this->generatorCollection[i]->generateNoise(xOffset / var7, yOffset / var7) * var7;
        var7 *= 2.0D;
    }

    return var5;
}

double InfdevOctaves::generateNoiseOctaves(double xOffset, double yOffset, double zOffset) {
    double var7 = 0.0D;
    double var9 = 1.0D;

    for(int i = 0; i < this->octaves; ++i) {
        var7 += this->generatorCollection[i]->generateNoiseD(xOffset / var9, yOffset / var9, zOffset / var9) * var9;
        var9 *= 2.0D;
    }

    return var7;
}

std::vector<double> InfdevOctaves::generateNoiseOctaves(std::vector<double> var1, double var2, double var4, double var6, int var8, int var9, int var10, double var11, double var13, double var15) {
    if(var1.empty()) {
        var1.reserve(var8 * var9 * var10);
    } else {
        for(int var17 = 0; var17 < var1.size(); ++var17) {
            var1[var17] = 0.0D;
        }
    }

    double var20 = 1.0D;

    for(int var19 = 0; var19 < this->octaves; ++var19) {
        // TODO: Add that
        //this.generatorCollection[var19].func_646_a(var1, var2, var4, var6, var8, var9, var10, var11 * var20, var13 * var20, var15 * var20, var20);
        var20 /= 2.0D;
    }

    return var1;
}

std::vector<double> InfdevOctaves::func_4103_a(std::vector<double> var1, int var2, int var3, int var4, int var5, double var6, double var8, double var10) {
    return this->generateNoiseOctaves(var1, (double)var2, 10.0D, (double)var3, var4, 1, var5, var6, 1.0D, var8);
}