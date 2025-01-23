#pragma once

#include "helper.h"
#include "items.h"

class Generator {
    private:
        virtual Block GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible = 0);
    public:
        int64_t seed;
        virtual Chunk GenerateChunk(int32_t x, int32_t z);
};