#pragma once

#include "helper.h"
#include "items.h"

class Generator {
    public:
        Chunk* GenerateChunk(int32_t x, int32_t z);
};