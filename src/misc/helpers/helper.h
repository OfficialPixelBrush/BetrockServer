#pragma once

#include <libdeflate.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <memory>

#include "packets.h"
#include "logger.h"
#include "blocks.h"
#include "items.h"
#include "directions.h"

#define CHUNK_HEIGHT 128
#define CHUNK_WIDTH_X 16
#define CHUNK_WIDTH_Z 16

#define CHUNK_DATA_SIZE static_cast<size_t>(CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z * 2.5)

// Item
struct Item {
    int16_t id = 0;
    int8_t  amount = 0;
    int16_t damage = 0;
};

// Building blocks
struct Block {
    uint8_t type = 0;
    uint8_t meta = 0;
    uint8_t lightBlock = 0;
    uint8_t lightSky = 0;
};

struct Chunk {
    struct Block blocks[CHUNK_WIDTH_X*CHUNK_WIDTH_Z*CHUNK_HEIGHT];
};

// Custom Types
struct Vec3 {
	double x,y,z;
    bool operator==(const Vec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    Vec3 operator+(const Vec3& other) const {
        return Vec3{x + other.x, y + other.y, z + other.z};
    }

    Vec3 operator-(const Vec3& other) const {
        return Vec3{x - other.x, y - other.y, z - other.z};
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec3& vec) {
        os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }
};

struct Int3 {
	int32_t x,y,z;
    bool operator==(const Int3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    Int3 operator+(const Int3& other) const {
        return Int3{x + other.x, y + other.y, z + other.z};
    }

    Int3 operator-(const Int3& other) const {
        return Int3{x - other.x, y - other.y, z - other.z};
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Int3& i) {
        os << "(" << i.x << ", " << i.y << ", " << i.z << ")";
        return os;
    }
};

typedef struct Vec3 Vec3;
typedef struct Int3 Int3;

// Converting between these types
Vec3 Int3ToVec3(Int3 i);
Int3 Vec3ToInt3(Vec3 v);
Int3 XyzToInt3(int32_t x, int32_t y, int32_t z);
bool Between(int value, int a, int b);

double GetDistance(Vec3 a, Vec3 b);
double GetDistance(Int3 a, Int3 b);

Int3 LocalToGlobalPosition(Int3 chunkPos, Int3 blockPos);
Int3 BlockToChunkPosition(Vec3 position);
Int3 BlockToChunkPosition(Int3 position);
void BlockToFace(int32_t& x, int8_t& y, int32_t& z, int8_t& direction);

// Converting a network response into a native type
int8_t EntryToByte(char* message, int32_t& offset);
int16_t EntryToShort(char* message, int32_t& offset);
int32_t EntryToInteger(char* message, int32_t& offset);
int64_t EntryToLong(char* message, int32_t& offset);
float EntryToFloat(char* message, int32_t& offset);
double EntryToDouble(char* message, int32_t& offset);
std::string EntryToString8(char* message, int32_t& offset);
std::string EntryToString16(char* message, int32_t& offset);

// Appending Data onto Network Response
void AppendShortToVector(std::vector<uint8_t> &vector, int16_t value);
void AppendIntegerToVector(std::vector<uint8_t> &vector, int32_t value);
void AppendLongToVector(std::vector<uint8_t> &vector, int64_t value);
void AppendFloatToVector(std::vector<uint8_t> &vector, float value);
void AppendDoubleToVector(std::vector<uint8_t> &vector, double value);
void AppendString8ToVector(std::vector<uint8_t> &vector, std::string value);
void AppendString16ToVector(std::vector<uint8_t> &vector, std::string value);

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
std::unique_ptr<char[]> CompressChunk(char* chunk, size_t &compressed_size);
std::unique_ptr<char[]> DecompressChunk(const char* compressed_data, size_t compressed_size, size_t& decompressed_size);

int64_t GetChunkHash(int32_t x, int32_t z);
Int3 DecodeChunkHash(int64_t hash);

int32_t SafeStringToInt(std::string in);
int64_t SafeStringToLong(std::string in);

int16_t GetMetaData(int32_t x, int8_t y, int32_t z, int8_t direction, int16_t id, int16_t damage);