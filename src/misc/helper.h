#pragma once

#include <libdeflate.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

#define CHUNK_HEIGHT 128
#define CHUNK_WIDTH_X 16
#define CHUNK_WIDTH_Z 16

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
};

struct Int3 {
	int32_t x,y,z;
};

typedef struct Vec3 Vec3;
typedef struct Int3 Int3;

// Converting between these types
Vec3 Int3ToVec3(Int3 i);
Int3 Vec3ToInt3(Vec3 v);
Int3 XyzToInt3(int32_t x, int32_t y, int32_t z);

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

// Packet Id Labels
std::string PacketIdToLabel(uint8_t id);

// Handling of Chunk and Block Data
int16_t GetBlockIndex(Int3 position);
char* CompressChunk(std::vector<uint8_t> chunk, size_t &compressed_size);