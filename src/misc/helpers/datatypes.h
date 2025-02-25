#pragma once
#include <iostream>

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

// TODO: Add a "modified" tag to a chunk to see if we need to bother re-saving it(?)
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