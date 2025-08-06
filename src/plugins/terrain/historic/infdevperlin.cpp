#include "infdevperlin.h"

InfdevPerlin::InfdevPerlin()
    : InfdevPerlin(new JavaRandom()) {}

InfdevPerlin::InfdevPerlin(JavaRandom* rand) {
    //this->permutations = new int[512];
    this->xCoord = rand->nextDouble() * 256.0D;
    this->yCoord = rand->nextDouble() * 256.0D;
    this->zCoord = rand->nextDouble() * 256.0D;

    for (int i = 0; i < 256; ++i) {
        this->permutations[i] = i;
    }

    for (int i = 0; i < 256; ++i) {
        int j = rand->nextInt(256 - i) + i;
        std::swap(this->permutations[i], this->permutations[j]);
        this->permutations[i + 256] = this->permutations[i];
    }
}

double InfdevPerlin::generateNoise(double xOffset, double yOffset, double zOffset) {
    double var7 = xOffset + this->xCoord;
    double var9 = yOffset + this->yCoord;
    double var11 = zOffset + this->zCoord;
    int var22 = (int)std::floor(var7) & 255;
    int var2 = (int)std::floor(var9) & 255;
    int var23 = (int)std::floor(var11) & 255;
    var7 -= (double)std::floor(var7);
    var9 -= (double)std::floor(var9);
    var11 -= (double)std::floor(var11);
    double var16 = staticGenerateNoise(var7);
    double var18 = staticGenerateNoise(var9);
    double var20 = staticGenerateNoise(var11);
    int var4 = this->permutations[var22] + var2;
    int var24 = this->permutations[var4] + var23;
    var4 = this->permutations[var4 + 1] + var23;
    var22 = this->permutations[var22 + 1] + var2;
    var2 = this->permutations[var22] + var23;
    var22 = this->permutations[var22 + 1] + var23;
    return lerp(var20, lerp(var18, lerp(var16, grad(this->permutations[var24], var7, var9, var11), grad(this->permutations[var2], var7 - 1.0D, var9, var11)), lerp(var16, grad(this->permutations[var4], var7, var9 - 1.0D, var11), grad(this->permutations[var22], var7 - 1.0D, var9 - 1.0D, var11))), lerp(var18, lerp(var16, grad(this->permutations[var24 + 1], var7, var9, var11 - 1.0D), grad(this->permutations[var2 + 1], var7 - 1.0D, var9, var11 - 1.0D)), lerp(var16, grad(this->permutations[var4 + 1], var7, var9 - 1.0D, var11 - 1.0D), grad(this->permutations[var22 + 1], var7 - 1.0D, var9 - 1.0D, var11 - 1.0D))));
}

double InfdevPerlin::generateNoise(double xOffset, double yOffset) {
    return this->generateNoise(xOffset, yOffset, 0.0D);
}

double InfdevPerlin::generateNoiseD(double xOffset, double yOffset, double zOffset) {
    return this->generateNoise(xOffset, yOffset, zOffset);
}