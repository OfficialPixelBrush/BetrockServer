#pragma once

#include "infdevperlin.h"
#include <vector>
#include <memory>

class InfdevOctaves {
    public:
        InfdevOctaves(int octaves);
        InfdevOctaves(JavaRandom& rand, int octaves);
        double generateNoise(double xOffset, double yOffset);
        double generateNoise(double xOffset, double yOffset, double zOffset);
    private:
        int octaves;
        std::vector<std::unique_ptr<InfdevPerlin>> generatorCollection;
};