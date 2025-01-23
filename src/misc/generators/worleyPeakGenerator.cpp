#include "worleyPeakGenerator.h"

// Up ahead is my own attempt at a world generator.
// Be warned, it may be terrible
// This is heavily baed on my Godot world generator

// Large prime numbers for mixing
const int64_t PRIME1 = 0x85297A4D;
const int64_t PRIME2 = 0x68E31DA4;
const int64_t PRIME3 = 0xB5297A4D;
const int64_t PRIME4 = 0x45D9F3B3;
	
// Bit mixing function
int64_t WorleyPeakGenerator::Mix(int64_t a , int64_t b) {
	return ((a ^ b) * PRIME1) & 0xFFFFFFFFFFFFFFFF;
}

int32_t WorleyPeakGenerator::SpatialPrng(Int3 position, int64_t seed) {
	int32_t x = position.x;
	int8_t y = position.y;
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

Int3 WorleyPeakGenerator::GetPointPositionInChunk(Int3 position) {
	int32_t spaceX = SpatialPrng(position, seed);
	int32_t spaceY = SpatialPrng(position, seed);
	int32_t spaceZ = SpatialPrng(position, seed);
	
	// Ensure positive modulo results
	int32_t x = ((spaceX ^ position.x) % CHUNK_WIDTH_X + CHUNK_WIDTH_X) % CHUNK_WIDTH_X;
	int32_t y = ((spaceY ^ position.y) % CHUNK_HEIGHT  + CHUNK_HEIGHT ) % CHUNK_HEIGHT;
	int32_t z = ((spaceZ ^ position.z) % CHUNK_WIDTH_Z + CHUNK_WIDTH_Z) % CHUNK_WIDTH_Z;
	
	return Int3 {x, y, z};
}

double WorleyPeakGenerator::FindDistanceToPoint(Int3 position) {
    Int3 chunkPos = {
        position.x>>4,
        0,
        position.z>>4,
    };
    Int3 blockPos = {
        position.x&0xF,
        position.y&0xF,
        position.z&0xF,
    };
    Block b;
    double smallestDistance = 10000.0f;

    for (int cX = -1; cX < 2; cX++) {
        for (int cZ = -1; cZ < 2; cZ++) {
            Int3 goalChunkPos = chunkPos;
            goalChunkPos.x += cX;
            goalChunkPos.y += 0;
            goalChunkPos.z += cZ;

            Int3 goalBlockPos = GetPointPositionInChunk(goalChunkPos);

            Int3 goalGlobalPos = LocalToGlobalPosition(goalChunkPos, goalBlockPos);
            double distance = GetDistance(position,goalGlobalPos);

            if (distance < smallestDistance) {
                smallestDistance = distance;
            } 
        }
    }
    return smallestDistance;
}

bool WorleyPeakGenerator::GetNoise(Int3 position) {
    double distance = FindDistanceToPoint(position);
    if (position.y < distance) {
        return true;
    }
    return false;
}

Block WorleyPeakGenerator::GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible) {
    Block b;
    if (position.y == 0) {
        b.type = BLOCK_BEDROCK;
    } else if (position.y > 0) {
        Int3 movedPos {
            position.x,
            position.y - 55,
            position.z
        };
        if (GetNoise(movedPos)) {
            if (blocksSinceSkyVisible == 0) {
                if (position.y > 66) {
                    b.type = BLOCK_GRASS;
                } else {
                    b.type = BLOCK_SAND;
                }
            } else if (blocksSinceSkyVisible > 0 && blocksSinceSkyVisible < 3) {
                if (position.y > 66) {
                    b.type = BLOCK_DIRT;
                } else {
                    b.type = BLOCK_SAND;
                }
            } else {
                b.type = BLOCK_STONE;
            }
        } else {
            if (position.y < 64) {
                b.type = BLOCK_WATER_STILL;
            }
        }
    }
    return b;
}