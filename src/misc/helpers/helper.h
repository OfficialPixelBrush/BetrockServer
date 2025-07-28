#pragma once

#include <algorithm>
#include <libdeflate.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <memory>
#include <array>

#include "PerlinNoise.hpp"

#include "packets.h"
#include "logger.h"
#include "blocks.h"
#include "items.h"
#include "datatypes.h"

Int3 LocalToGlobalPosition(Int3 chunkPos, Int3 blockPos);
Int3 BlockToChunkPosition(Vec3 position);
Int3 BlockToChunkPosition(Int3 position);

int8_t ConvertFloatToPackedByte(float value);

bool Between(int value, int a, int b);

double GetEuclidianDistance(Vec3 a, Vec3 b);
double GetEuclidianDistance(Int3 a, Int3 b);
double GetTaxicabDistance(Vec3 a, Vec3 b);
double GetTaxicabDistance(Int3 a, Int3 b);
double GetChebyshevDistance(Vec3 a, Vec3 b);
double GetChebyshevDistance(Int3 a, Int3 b);

// Packet Id Labels
std::string PacketIdToLabel(Packet packet);

// Handling of Chunk and Block Data
int16_t GetBlockIndex(Int3 position);
Int3 GetBlockPosition(int index);
std::unique_ptr<char[]> CompressChunk(char* chunk, size_t &compressed_size);
std::unique_ptr<char[]> DecompressChunk(const char* compressed_data, size_t compressed_size, size_t& decompressed_size);

int64_t GetChunkHash(int32_t x, int32_t z);
Int3 DecodeChunkHash(int64_t hash);

int32_t SafeStringToInt(std::string in);
int64_t SafeStringToLong(std::string in);

std::string GetRealTime();
std::string GetRealTimeFileFormat();

std::string Uint8ArrayToHexDump(const uint8_t* array, size_t size);

void LimitBlockCoordinates(Int3 &position);

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