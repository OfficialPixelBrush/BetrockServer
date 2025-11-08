// A recreation of the the Infdev 20100227-1433 Perlin noise function
#pragma once
#include "noiseGenerator.h"

class NoiseSimplex : public NoiseGenerator {
    private:
        int permutations[512];
        double xCoord;
        double yCoord;
        double zCoord;
        double GenerateNoiseBase(double x, double y, double z);
        int gradients[12][3] = {
            {1, 1, 0},
            {-1, 1, 0},
            {1, -1, 0},
            {-1, -1, 0},
            {1, 0, 1},
            {-1, 0, 1},
            {1, 0, -1},
            {-1, 0, -1},
            {0, 1, 1},
            {0, -1, 1},
            {0, 1, -1},
            {0, -1, -1}
        };
        double skewing = 0.5D * (sqrt(3.0D) - 1.0D);
        double unskewing = (3.0D - sqrt(3.0D)) / 6.0D;
    public:
        NoiseSimplex();
        NoiseSimplex(JavaRandom* rand);
        void GenerateNoise(
            std::vector<double>& noiseField,
            double xOffset, double yOffset,
            int width, int height,
            double xScale, double yScale,
            double amplitude
        );
};

inline int wrap(double grad) {
    return grad > 0.0D ? (int)grad : (int)grad - 1;
}

inline double dotProd(int grad[3], double x, double y) {
    return (double)grad[0] * x + (double)grad[1] * y;
}