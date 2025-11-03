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
#include "chunk.h"

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

template <typename T>
void PrintVector(std::vector<T>& values) {
    std::cout << "[";
    for (size_t i = 0; i < values.size(); i++) {
        std::cout << values[i];
        if (i < values.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << "]" << std::endl;
}

size_t GetFileSize(std::fstream& file);