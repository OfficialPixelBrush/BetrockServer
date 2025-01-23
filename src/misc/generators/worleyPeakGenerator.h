#pragma once
#include <cstdint>
#include "generator.h"

class WorleyPeakGenerator : public Generator {
    private:
        int64_t Mix(int64_t a , int64_t b);
        int32_t SpatialPrng(Int3 position, int64_t seed);
        Int3 GetPointPositionInChunk(Int3 position);
        double FindDistanceToPoint(Int3 position);
        bool GetNoise(Int3 position);
        Block GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible) override;
};