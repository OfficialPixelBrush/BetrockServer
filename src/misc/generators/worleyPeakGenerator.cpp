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

int32_t WorleyPeakGenerator::SpatialPrng(Int3 position) {
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

Int3 WorleyPeakGenerator::GetPointPositionInChunk(Int3 position, float verticalScale) {
    // Use different seeds for x, y, z to ensure unique coordinates
    int32_t randomized = SpatialPrng(position);

    int32_t x = ((randomized ^ position.x) % CHUNK_WIDTH_X + CHUNK_WIDTH_X) % CHUNK_WIDTH_X;
    int32_t y = ((randomized ^ position.y) % CHUNK_HEIGHT  + CHUNK_HEIGHT ) % CHUNK_HEIGHT;
    y = y*verticalScale;
    int32_t z = ((randomized ^ position.z) % CHUNK_WIDTH_Z + CHUNK_WIDTH_Z) % CHUNK_WIDTH_Z;
    
    return Int3{x, y, z};
}

double WorleyPeakGenerator::FindDistanceToPoint(Int3 position, float verticalScale) {
    Int3 chunkPos = {
        position.x >> 4,
        0,
        position.z >> 4
    };
    
    double smallestDistance = std::numeric_limits<double>::max();
    
    // Check neighboring chunks horizontally
    for (int cX = -1; cX < 2; cX++) {
        for (int cZ = -1; cZ < 2; cZ++) {
            Int3 goalChunkPos = chunkPos;
            goalChunkPos.x += cX;
            goalChunkPos.z += cZ;
            
            Int3 goalBlockPos = GetPointPositionInChunk(goalChunkPos, verticalScale);
            Int3 goalGlobalPos = LocalToGlobalPosition(goalChunkPos, goalBlockPos);
            
            double distance = GetDistance(position, goalGlobalPos);
            smallestDistance = std::min(smallestDistance, distance);
        }
    }
    
    return smallestDistance;
}

double smoothstep(double edge0, double edge1, double x) {
    // Clamp x between 0 and 1
    double t = std::max(0.0, std::min(1.0, (x - edge0) / (edge1 - edge0)));
    // Cubic interpolation for smoother transition
    return t * t * (3.0 - 2.0 * t);
}

double WorleyPeakGenerator::GetNoise(Int3 position, double threshold, float verticalScale) {
    double distance = FindDistanceToPoint(position, verticalScale);
    
    // Use smoothstep for more natural falloff
    return 1.0 - smoothstep(0.0, threshold, distance);
}

Block WorleyPeakGenerator::GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible) {
    Block b;
    bool solid = true;
    if (position.y == 0) {
        b.type = BLOCK_BEDROCK;
    } else if (position.y > 0) {
        if (Between(position.y,0,3) && SpatialPrng(position)%2) {
            b.type = BLOCK_BEDROCK;
            return b;
        }
        if (GetNoise(Int3(position.x, CHUNK_HEIGHT - position.y - 13, position.z), 60.0) > 0.2) {
            //std::cout << position << ": " << GetNoise(Int3{position.x,0,position.z}, 1) << std::endl;
            solid = false;
        } else {
            if (position.y < 64) {
                b.type = BLOCK_WATER_STILL;
            }
        }
    }

    if (solid) {
        if (blocksSinceSkyVisible == 0) {
            if (position.y > (64 + (SpatialPrng(position)%2))) {
                b.type = BLOCK_GRASS;
            } else {
                b.type = BLOCK_SAND;
            }
        } else if (blocksSinceSkyVisible > 0 && blocksSinceSkyVisible < 3+(SpatialPrng(position)%2)) {
            if (position.y > (64 + (SpatialPrng(position)%2))) {
                b.type = BLOCK_DIRT;
            } else {
                b.type = BLOCK_SAND;
            }
        } else {
            if (Between(position.y,5,52) && SpatialPrng(position)%CHANCE_COAL==0) {
                b.type = BLOCK_ORE_COAL;
            } else if (Between(position.y,13,17) && SpatialPrng(position)%CHANCE_LAPIS_LAZULI==0) {
                b.type = BLOCK_ORE_LAPIS_LAZULI;
            } else if (Between(position.y,5,54) && SpatialPrng(position)%CHANCE_IRON==0) {
                b.type = BLOCK_ORE_IRON;
            } else if (Between(position.y,5,29) && SpatialPrng(position)%CHANCE_GOLD==0) {
                b.type = BLOCK_ORE_GOLD;
            } else if (Between(position.y,5,12) && SpatialPrng(position)%CHANCE_DIAMOND==0) {
                b.type = BLOCK_ORE_DIAMOND;
            } else if (Between(position.y,5,12) && SpatialPrng(position)%CHANCE_REDSTONE==0) {
                b.type = BLOCK_ORE_REDSTONE;
            } else {
                b.type = BLOCK_STONE;
            }
        }
    } else {
        if (position.y < 64) {
            b.type = BLOCK_WATER_STILL;
        }
    }
    return b;
}