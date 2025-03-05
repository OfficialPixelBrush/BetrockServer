#pragma once
#include <iostream>

#define CHUNK_HEIGHT 128
#define CHUNK_WIDTH_X 16
#define CHUNK_WIDTH_Z 16

#define CHUNK_DATA_SIZE static_cast<size_t>(CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z * 2.5)

#define OLD_CHUNK_FILE_EXTENSION ".cnk"
#define CHUNK_FILE_EXTENSION ".ncnk"

// Item
struct Item {
    int16_t id = 0;
    int8_t  amount = 0;
    int16_t damage = 0; // Also known as metadata

    friend std::ostream& operator<<(std::ostream& os, const Item& i) {
        os << "(" << (int)i.id << ":" << (int)i.damage << "x" << (int)i.amount << ")";
        return os;
    }
};

// Building blocks
struct Block {
    int8_t type = 0;
    int8_t meta = 0;
    int8_t lightBlock = 0;
    int8_t lightSky = 0;

    friend std::ostream& operator<<(std::ostream& os, const Block& b) {
        os << "(" << (int)b.type << ":" << (int)b.meta << ")";
        return os;
    }
};

// TODO: Add a "modified" tag to a chunk to see if we need to bother re-saving it(?)
struct Chunk {
    struct Block blocks[CHUNK_WIDTH_X*CHUNK_WIDTH_Z*CHUNK_HEIGHT];
    // This describes the number of clients that can see this chunk.
    // If this hits 0, the chunk is invisible and can be removed
    // TODO: Actually implement this value!
    uint16_t viewers = 0;

    // A non-populated chunk still needs to be popualated with foliage
    bool populated = false;
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

Vec3 Int3ToVec3(Int3 i);
Int3 Vec3ToInt3(Vec3 v);

Int3 Int3ToEntityInt3(Int3 pos);
Int3 Vec3ToEntityInt3(Vec3 pos);
Vec3 EntityInt3ToVec3(Int3 pos);