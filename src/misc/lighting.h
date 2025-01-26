#pragma once
#include <cstdint>
#include "helper.h"
#include "items.h"

void CalculateColumnLight(int32_t x, int32_t z, Chunk* c);
void CalculateChunkLight(Chunk* c);