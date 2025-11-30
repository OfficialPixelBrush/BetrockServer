#pragma once
#include <iostream>
#include <string>
#include <sstream>

#define CHUNK_HEIGHT 128
#define CHUNK_WIDTH_X 16
#define CHUNK_WIDTH_Z 16
#define WATER_LEVEL CHUNK_HEIGHT/2

#define CHUNK_DATA_SIZE static_cast<size_t>(CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z * 2.5)

#define OLD_CHUNK_FILE_EXTENSION ".cnk"
#define CHUNK_FILE_EXTENSION ".ncnk"
#define MCREGION_FILE_EXTENSION ".mcr"

// Item
struct Item {
    int16_t id = -1;
    int8_t  amount = 0;
    int16_t damage = 0; // Also known as metadata

    friend std::ostream& operator<<(std::ostream& os, const Item& i) {
        os << "(" << int(i.id) << ":" << int(i.damage) << "x" << int(i.amount) << ")";
        return os;
    }
    
    std::string str() const {
        std::ostringstream oss;
        oss << *this; // Use the overloaded << operator
        return oss.str();
    }
};

// Block Struct
struct Block {
    int8_t type = 0;
    int8_t meta = 0;
    int8_t blocklight = 0;
    int8_t skylight = 0;

    friend std::ostream& operator<<(std::ostream& os, const Block& b) {
        os << "(" << int(b.type) << ":" << int(b.meta) << ")";
        return os;
    }
    
    std::string str() const {
        std::ostringstream oss;
        oss << *this; // Use the overloaded << operator
        return oss.str();
    }
};

// Custom Types
/**
 * @brief A struct that contains 3 doubles (x,y,z)
 * 
 */
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
    
    std::string str() const {
        std::ostringstream oss;
        oss << *this; // Use the overloaded << operator
        return oss.str();
    }

    double& operator[](int i) {
        return *(&x + i);
    }
    
    const double& operator[](int i) const {
        return *(&x + i);
    }
};

/**
 * @brief A struct that contains 2 doubles (x,y)
 * 
 */
struct Vec2 {
	double x,y;
    bool operator==(const Vec2& other) const {
        return x == other.x && y == other.y;
    }

    Vec2 operator+(const Vec2& other) const {
        return Vec2{x + other.x, y + other.y};
    }

    Vec2 operator-(const Vec2& other) const {
        return Vec2{x - other.x, y - other.y};
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec2& vec) {
        os << "(" << vec.x << ", " << vec.y << ")";
        return os;
    }
    
    std::string str() const {
        std::ostringstream oss;
        oss << *this; // Use the overloaded << operator
        return oss.str();
    }

    double& operator[](int i) {
        return *(&x + i);
    }
    
    const double& operator[](int i) const {
        return *(&x + i);
    }
};

/**
 * @brief A struct that contains 3 integers (x,y,z)
 * 
 */
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
    
    std::string str() const {
        std::ostringstream oss;
        oss << *this; // Use the overloaded << operator
        return oss.str();
    }

    int32_t& operator[](int32_t i) {
        return *(&x + i);
    }

    const int32_t& operator[](int32_t i) const {
        return *(&x + i);
    }
};


/**
 * @brief A struct that contains 2 integers (x,y)
 * 
 */
struct Int2 {
	int32_t x,y;
    bool operator==(const Int2& other) const {
        return x == other.x && y == other.y;
    }

    Int2 operator+(const Int2& other) const {
        return Int2{x + other.x, y + other.y};
    }

    Int2 operator-(const Int2& other) const {
        return Int2{x - other.x, y - other.y};
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Int2& i) {
        os << "(" << i.x << ", " << i.y << ")";
        return os;
    }
    
    std::string str() const {
        std::ostringstream oss;
        oss << *this; // Use the overloaded << operator
        return oss.str();
    }

    int32_t& operator[](int32_t i) {
        return *(&x + i);
    }

    const int32_t& operator[](int32_t i) const {
        return *(&x + i);
    }
};

/**
 * @brief Axis-aligned Bounding Box
 * 
 */
struct AABB {
    Vec3 min;
    Vec3 max;
};

typedef struct Vec3 Vec3;
typedef struct Int3 Int3;
typedef struct AABB AABB;

Vec3 Int3ToVec3(Int3 i);
Int3 Vec3ToInt3(Vec3 v);

Int3 Int3ToEntityInt3(Int3 pos);
Int3 Vec3ToEntityInt3(Vec3 pos);
Vec3 EntityInt3ToVec3(Int3 pos);
AABB CalculateAABB(Vec3 position, AABB base);