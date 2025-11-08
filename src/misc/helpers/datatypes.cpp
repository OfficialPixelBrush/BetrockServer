#include "datatypes.h"

// Converting between these types
// Convert an Int3 into a Vec3
Vec3 Int3ToVec3(Int3 i) {
	Vec3 v = {
		(double)i.x+0.5,
		(double)i.y,
		(double)i.z+0.5
	};
	return v;
}

// Convert an Vec3 into a Int3
Int3 Vec3ToInt3(Vec3 v) {
	Int3 i = {
		(int32_t)v.x,
		(int32_t)v.y,
		(int32_t)v.z
	};
	return i;
}

// Translate a block space position to entity space
Int3 Int3ToEntityInt3(Int3 pos) {
	return Int3 {
		pos.x << 5 | 16,
		pos.y << 5 | 16,
		pos.z << 5 | 16
	};
}

// Translate a player space position to entity space
Int3 Vec3ToEntityInt3(Vec3 pos) {
	return Int3 {
		int32_t(pos.x*32),
		int32_t(pos.y*32),
		int32_t(pos.z*32)
	};
}

// Translate an entity space position to player space
Vec3 EntityInt3ToVec3(Int3 pos) {
	return Vec3 {
		double(pos.x)/32,
		double(pos.y)/32,
		double(pos.z)/32
	};
}

AABB CalculateAABB(Vec3 position, AABB base) {
    return AABB {
		// This should be negative
        position + base.min,
		// This should be positive
        position + base.max
    };
}