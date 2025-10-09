#include "infdevperlin.h"

InfdevPerlin::InfdevPerlin()
    : InfdevPerlin(new JavaRandom()) {}

InfdevPerlin::InfdevPerlin(JavaRandom* rand) {
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

// This is a rather standard implementation of "Improved Perlin Noise",
// as described by Ken Perlin in 2002
double InfdevPerlin::generateNoise(double x, double y, double z) {
    x += this->xCoord;
    y += this->yCoord;
    z += this->zCoord;
    // The farlands are caused by this getting cast to a 32-Bit Integer.
    // Change these ints to longs to fix the farlands.
    int xInt = (int)x;
    int yInt = (int)y;
    int zInt = (int)z;
    if(x < (double)xInt) --xInt;
    if(y < (double)yInt) --yInt;
    if(z < (double)zInt) --zInt;

    int xIndex = xInt & 255;
    int yIndex = yInt & 255;
    int zIndex = zInt & 255;

    x -= (double)xInt;
    y -= (double)yInt;
    z -= (double)zInt;
    double w = fade(x);
    double v = fade(y);
    double u = fade(z);
    int permXY = this->permutations[xIndex] + yIndex;
    int permXYZ = this->permutations[permXY] + zIndex;
    // Some of the following code is weird,
    // probably because it got optimized by Java to use
    // fewer variables or Notch did this to be efficient
    permXY = this->permutations[permXY + 1] + zIndex;
    xIndex = this->permutations[xIndex + 1] + yIndex;
    yIndex = this->permutations[xIndex] + zIndex;
    xIndex = this->permutations[xIndex + 1] + zIndex;
    return lerp(u,
        lerp(v,
            lerp(w,
                grad(this->permutations[permXYZ], x, y, z),
                grad(this->permutations[yIndex], x - 1.0D, y, z)
            ), lerp(w,
                grad(this->permutations[permXY], x, y - 1.0D, z),
                grad(this->permutations[xIndex], x - 1.0D, y - 1.0D, z)
            )
        ), lerp(v,
                lerp(w, grad(this->permutations[permXYZ + 1], x, y, z - 1.0D),
                grad(this->permutations[yIndex + 1], x - 1.0D, y, z - 1.0D)
            ), lerp(w,
                grad(this->permutations[permXY + 1], x, y - 1.0D, z - 1.0D),
                grad(this->permutations[xIndex + 1], x - 1.0D, y - 1.0D, z - 1.0D)
            )
        )
    );
}

double InfdevPerlin::generateNoise(double x, double y) {
    return this->generateNoise(x, y, 0.0D);
}

double InfdevPerlin::generateNoiseD(double x, double y, double z) {
    return this->generateNoise(x, y, z);
}