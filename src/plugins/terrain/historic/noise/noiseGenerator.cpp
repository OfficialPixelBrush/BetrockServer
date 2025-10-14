#include "noiseGenerator.h"

NoiseGenerator::NoiseGenerator() {}

NoiseGenerator::NoiseGenerator(JavaRandom* rand) {
    xCoord = yCoord = zCoord = 0.0;
}

double NoiseGenerator::GenerateNoise(double x, double y) {
    return 0;
}

double NoiseGenerator::GenerateNoise(double x, double y, double z) {
    return 0;
}
