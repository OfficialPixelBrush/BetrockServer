#pragma once
#include <cstdint>
#include <cmath>
#include "datatypes.h"
#include "helper.h"

#include "PerlinNoise.hpp"
#include "blocks.h"

const siv::PerlinNoise::seed_type seedp = 0;
const siv::PerlinNoise perlin{ seedp };

// --- TERRAIN GEN RELATED ---
int64_t Mix(int64_t a , int64_t b);
int32_t SpatialPrng(int64_t seed, Int3 position);
Int3 GetPointPositionInChunk(int64_t seed, Int3 position, Vec3 scale);
double FindDistanceToPoint(int64_t seed, Int3 position, Vec3 scale);
double SmoothStep(double edge0, double edge1, double x);
double GetNoiseWorley(int64_t seed, Int3 position, double threshold, Vec3 scale);
double GetNoisePerlin2D(int64_t seed, Vec3 position, int octaves);
double GetNoisePerlin3D(int64_t seed, Vec3 position, int octaves);
Block GetNaturalGrass(int64_t seed, Int3 position, int32_t blocksSinceSkyVisible);