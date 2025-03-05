#pragma once
#include <cstdint>
#include "helper.h"
#include "blocks.h"
#include "world.h"
#include "datatypes.h"

void CalculateColumnLight(int32_t x, int32_t z, Chunk* c, int32_t& unobstructedLayers);
void CalculateChunkLight(Chunk* c);
void CalculateSpreadLight(Chunk* c);
void PropagateLight(World* world, Int3 position, int8_t lightLevel, bool source = false);