#include "terrainhelper.h"

// Large prime numbers for mixing
const int64_t PRIME1 = 0x85297A4D;
const int64_t PRIME2 = 0x68E31DA4;
const int64_t PRIME3 = 0xB5297A4D;
const int64_t PRIME4 = 0x45D9F3B3;

// Bit mixing function
int64_t Mix(int64_t a , int64_t b) {
	return ((a ^ b) * PRIME1) & 0xFFFFFFFFFFFFFFFF;
}

int32_t SpatialPrng(int64_t seed, Int3 position) {
	int32_t x = position.x;
	int32_t y = position.y;
	int32_t z = position.z;
	// Initial hash
	int64_t h = seed & 0xFFFFFFFF;
	
	// Mix X coordinate
	h = Mix(h, x * PRIME1);
	h = ((h << 13) | (h >> 19)) & 0xFFFFFFFF;
	
	// Mix Y coordinate
	h = Mix(h, y * PRIME2);
	h = ((h << 17) | (h >> 15)) & 0xFFFFFFFF;
	
	// Mix Z coordinate
	h = Mix(h, z * PRIME3);
	h = ((h << 11) | (h >> 21)) & 0xFFFFFFFF;
	
	// Final mixing
	h = Mix(h, h >> 16);
	h = Mix(h, h >> 8);
	h *= PRIME4;
	h ^= h >> 11;
	h *= PRIME1;
	
	return h & 0x7FFFFFFF; //Ensure positive number
}

Int3 GetPointPositionInChunk(int64_t seed, Int3 position, Vec3 scale) {
    // Use different seeds for x, y, z to ensure unique coordinates
    int32_t randomized = SpatialPrng(seed, position);

    int32_t x = int32_t(double(((randomized ^ position.x) % CHUNK_WIDTH_X + CHUNK_WIDTH_X) % CHUNK_WIDTH_X) * scale.x);
    int32_t y = int32_t(double(((randomized ^ position.y) % CHUNK_HEIGHT  + CHUNK_HEIGHT ) % CHUNK_HEIGHT ) * scale.y);
    int32_t z = int32_t(double(((randomized ^ position.z) % CHUNK_WIDTH_Z + CHUNK_WIDTH_Z) % CHUNK_WIDTH_Z) * scale.z);
    
    return Int3{x, y, z};
}

double FindDistanceToPoint(int64_t seed, Int3 position, Vec3 scale) {
    Int3 chunkPos = {
        position.x >> 4,
        0,
        position.z >> 4
    };
    
    double smallestDistance = std::numeric_limits<double>::max();
    
    // Check neighboring chunks horizontally
    for (int32_t cX = -1; cX < 2; cX++) {
        for (int32_t cZ = -1; cZ < 2; cZ++) {
            Int3 goalChunkPos = chunkPos;
            goalChunkPos.x += cX;
            goalChunkPos.z += cZ;
            
            Int3 goalBlockPos = GetPointPositionInChunk(seed, goalChunkPos, scale);
            Int3 goalGlobalPos = LocalToGlobalPosition(goalChunkPos, goalBlockPos);
            
            double distance = GetEuclidianDistance(position, goalGlobalPos);
            smallestDistance = std::min(smallestDistance, distance);
        }
    }
    
    return smallestDistance;
}

double SmoothStep(double edge0, double edge1, double x) {
    // Clamp x between 0 and 1
    double t = std::max(0.0, std::min(1.0, (x - edge0) / (edge1 - edge0)));
    // Cubic interpolation for smoother transition
    return t * t * (3.0 - 2.0 * t);
}

double GetNoiseWorley(int64_t seed, Int3 position, double threshold, Vec3 scale) {
    double distance = FindDistanceToPoint(seed, position, scale);
    
    // Use smoothstep for more natural falloff
    return 1.0 - SmoothStep(0.0, threshold, distance);
}

double GetNoisePerlin2D([[maybe_unused]] int64_t seed, Vec3 position, int32_t octaves) {
    return perlin.octave2D_01(position.x, position.z, octaves);
}

double GetNoisePerlin3D([[maybe_unused]] int64_t seed, Vec3 position, int32_t octaves) {
    return perlin.octave3D_01(position.x, position.y, position.z, octaves);
}

Block GetNaturalGrass(int64_t seed, Int3 position, int32_t blocksSinceSkyVisible) {
    Block b;
    if (blocksSinceSkyVisible == 0) {
        b.type = BLOCK_GRASS;
    } else if (Between(blocksSinceSkyVisible,0,3 + (SpatialPrng(seed,position)%2))) {
        b.type = BLOCK_DIRT;
    } else {
        b.type = BLOCK_STONE;
    }
    return b;
}