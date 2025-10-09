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

void InfdevPerlin::func_646_a(std::vector<double>& var1, double var2, double var4, double var6, int var8, int var9, int var10, double var11, double var13, double var15, double var17) {
    int var10001;
    int var19;
    int var22;
    double var31;
    double var35;
    int var37;
    double var38;
    int var40;
    int var41;
    double var42;
    int var75;
    if(var9 == 1) {
        bool var64 = false;
        bool var65 = false;
        bool var21 = false;
        bool var68 = false;
        double var70 = 0.0D;
        double var73 = 0.0D;
        var75 = 0;
        double var77 = 1.0D / var17;

        for(int var30 = 0; var30 < var8; ++var30) {
            var31 = (var2 + (double)var30) * var11 + this->xCoord;
            int var78 = (int)var31;
            if(var31 < (double)var78) {
                --var78;
            }

            int var34 = var78 & 255;
            var31 -= (double)var78;
            var35 = var31 * var31 * var31 * (var31 * (var31 * 6.0D - 15.0D) + 10.0D);

            for(var37 = 0; var37 < var10; ++var37) {
                var38 = (var6 + (double)var37) * var15 + this->zCoord;
                var40 = (int)var38;
                if(var38 < (double)var40) {
                    --var40;
                }

                var41 = var40 & 255;
                var38 -= (double)var40;
                var42 = var38 * var38 * var38 * (var38 * (var38 * 6.0D - 15.0D) + 10.0D);
                var19 = this->permutations[var34] + 0;
                int var66 = this->permutations[var19] + var41;
                int var67 = this->permutations[var34 + 1] + 0;
                var22 = this->permutations[var67] + var41;
                var70 = lerp(var35, altGrad(this->permutations[var66], var31, var38), grad(this->permutations[var22], var31 - 1.0D, 0.0D, var38));
                var73 = lerp(var35, grad(this->permutations[var66 + 1], var31, 0.0D, var38 - 1.0D), grad(this->permutations[var22 + 1], var31 - 1.0D, 0.0D, var38 - 1.0D));
                double var79 = lerp(var42, var70, var73);
                var10001 = var75++;
                var1[var10001] += var79 * var77;
            }
        }

    } else {
        var19 = 0;
        double var20 = 1.0D / var17;
        var22 = -1;
        bool var23 = false;
        bool var24 = false;
        bool var25 = false;
        bool var26 = false;
        bool var27 = false;
        bool var28 = false;
        double var29 = 0.0D;
        var31 = 0.0D;
        double var33 = 0.0D;
        var35 = 0.0D;

        for(var37 = 0; var37 < var8; ++var37) {
            var38 = (var2 + (double)var37) * var11 + this->xCoord;
            var40 = (int)var38;
            if(var38 < (double)var40) {
                --var40;
            }

            var41 = var40 & 255;
            var38 -= (double)var40;
            var42 = var38 * var38 * var38 * (var38 * (var38 * 6.0D - 15.0D) + 10.0D);

            for(int var44 = 0; var44 < var10; ++var44) {
                double var45 = (var6 + (double)var44) * var15 + this->zCoord;
                int var47 = (int)var45;
                if(var45 < (double)var47) {
                    --var47;
                }

                int var48 = var47 & 255;
                var45 -= (double)var47;
                double var49 = var45 * var45 * var45 * (var45 * (var45 * 6.0D - 15.0D) + 10.0D);

                for(int var51 = 0; var51 < var9; ++var51) {
                    double var52 = (var4 + (double)var51) * var13 + this->yCoord;
                    int var54 = (int)var52;
                    if(var52 < (double)var54) {
                        --var54;
                    }

                    int var55 = var54 & 255;
                    var52 -= (double)var54;
                    double var56 = var52 * var52 * var52 * (var52 * (var52 * 6.0D - 15.0D) + 10.0D);
                    if(var51 == 0 || var55 != var22) {
                        var22 = var55;
                        int var69 = this->permutations[var41] + var55;
                        int var71 = this->permutations[var69] + var48;
                        int var72 = this->permutations[var69 + 1] + var48;
                        int var74 = this->permutations[var41 + 1] + var55;
                        var75 = this->permutations[var74] + var48;
                        int var76 = this->permutations[var74 + 1] + var48;
                        var29 = lerp(var42, grad(this->permutations[var71], var38, var52, var45), grad(this->permutations[var75], var38 - 1.0D, var52, var45));
                        var31 = lerp(var42, grad(this->permutations[var72], var38, var52 - 1.0D, var45), grad(this->permutations[var76], var38 - 1.0D, var52 - 1.0D, var45));
                        var33 = lerp(var42, grad(this->permutations[var71 + 1], var38, var52, var45 - 1.0D), grad(this->permutations[var75 + 1], var38 - 1.0D, var52, var45 - 1.0D));
                        var35 = lerp(var42, grad(this->permutations[var72 + 1], var38, var52 - 1.0D, var45 - 1.0D), grad(this->permutations[var76 + 1], var38 - 1.0D, var52 - 1.0D, var45 - 1.0D));
                    }

                    double var58 = lerp(var56, var29, var31);
                    double var60 = lerp(var56, var33, var35);
                    double var62 = lerp(var49, var58, var60);
                    var10001 = var19++;
                    var1[var10001] += var62 * var20;
                }
            }
        }

    }
}