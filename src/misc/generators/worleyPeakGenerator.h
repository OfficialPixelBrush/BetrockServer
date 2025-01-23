#pragma once
#include <cstdint>
#include "generator.h"

#define CHANCE_COAL 7
#define CHANCE_LAPIS_LAZULI 13
#define CHANCE_IRON 10
#define CHANCE_GOLD 15
#define CHANCE_DIAMOND 12
#define CHANCE_REDSTONE 11

class WorleyPeakGenerator : public Generator {
    private:
        int64_t Mix(int64_t a , int64_t b);
        int32_t SpatialPrng(Int3 position);
        Int3 GetPointPositionInChunk(Int3 position, float verticalScale = 0.1);
        double FindDistanceToPoint(Int3 position, float verticalScale = 0.1);
        double GetNoise(Int3 position, double threshold = 20.0, float verticalScale = 0.1);
        Block GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible) override;
};