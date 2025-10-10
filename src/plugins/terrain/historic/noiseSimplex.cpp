#include "noiseSimplex.h"

NoiseSimplex::NoiseSimplex()
    : NoiseSimplex(new JavaRandom()) {}

NoiseSimplex::NoiseSimplex(JavaRandom* rand) {
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

void NoiseSimplex::GenerateNoise(std::vector<double>& var1, double var2, double var4, int var6, int var7, double var8, double var10, double var12) {
    int var14 = 0;

    for(int var15 = 0; var15 < var6; ++var15) {
        double var16 = (var2 + (double)var15) * var8 + this->xCoord;

        for(int var18 = 0; var18 < var7; ++var18) {
            double var19 = (var4 + (double)var18) * var10 + this->yCoord;
            double var27 = (var16 + var19) * field_4315_f;
            int var29 = wrap(var16 + var27);
            int var30 = wrap(var19 + var27);
            double var31 = (double)(var29 + var30) * field_4314_g;
            double var33 = (double)var29 - var31;
            double var35 = (double)var30 - var31;
            double var37 = var16 - var33;
            double var39 = var19 - var35;
            uint8_t var41;
            uint8_t var42;
            if(var37 > var39) {
                var41 = 1;
                var42 = 0;
            } else {
                var41 = 0;
                var42 = 1;
            }

            double var43 = var37 - (double)var41 + field_4314_g;
            double var45 = var39 - (double)var42 + field_4314_g;
            double var47 = var37 - 1.0D + 2.0D * field_4314_g;
            double var49 = var39 - 1.0D + 2.0D * field_4314_g;
            int var51 = var29 & 255;
            int var52 = var30 & 255;
            int var53 = this->permutations[var51 + this->permutations[var52]] % 12;
            int var54 = this->permutations[var51 + var41 + this->permutations[var52 + var42]] % 12;
            int var55 = this->permutations[var51 + 1 + this->permutations[var52 + 1]] % 12;
            double var56 = 0.5D - var37 * var37 - var39 * var39;
            double var21;
            if(var56 < 0.0D) {
                var21 = 0.0D;
            } else {
                var56 *= var56;
                var21 = var56 * var56 * func_4114_a(gradients[var53], var37, var39);
            }

            double var58 = 0.5D - var43 * var43 - var45 * var45;
            double var23;
            if(var58 < 0.0D) {
                var23 = 0.0D;
            } else {
                var58 *= var58;
                var23 = var58 * var58 * func_4114_a(gradients[var54], var43, var45);
            }

            double var60 = 0.5D - var47 * var47 - var49 * var49;
            double var25;
            if(var60 < 0.0D) {
                var25 = 0.0D;
            } else {
                var60 *= var60;
                var25 = var60 * var60 * func_4114_a(gradients[var55], var47, var49);
            }

            int var10001 = var14++;
            var1[var10001] += 70.0D * (var21 + var23 + var25) * var12;
        }
    }
}