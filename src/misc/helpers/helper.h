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

#include "packets.h"
#include "logger.h"
#include "blocks.h"
#include "items.h"
#include "datatypes.h"

Int3 LocalToGlobalPosition(Int3 chunkPos, Int3 blockPos);
Int3 BlockToChunkPosition(Vec3 position);
Int3 BlockToChunkPosition(Int3 position);

int8_t ConvertFloatToPackedByte(float value);
Vec3 SubtractVec3(Vec3 previousPosition, Vec3 currentPosition);
Int3 Vec3ToRelativeInt3(Vec3 previousPosition, Vec3 currentPosition);

Int3 Int3ToEntityInt3(Int3 pos);
Int3 Vec3ToEntityInt3(Vec3 pos);
Vec3 EntityInt3ToVec3(Int3 pos);

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