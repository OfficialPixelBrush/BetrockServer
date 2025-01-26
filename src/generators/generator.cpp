#include "generator.h"

void Generator::PrepareGenerator(int64_t seed) {
    this->seed = seed;
    L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushnumber(L, seed);
    lua_setglobal(L, "seed");

    // Add helper functions
    lua_register(L,"between", lua_Between);
    lua_register(L,"spatialPrng", lua_SpatialPRNG);
    lua_register(L,"getNoiseWorley", lua_GetNoiseWorley);
    lua_register(L,"getNaturalGrass", lua_GetNaturalGrass);
    
    // Execute a Lua script
    if (luaL_dofile(L, ("scripts/" + properties["generator"]).c_str())) {
        std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
        lua_close(L);
    }
}

Block Generator::GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible) {
    Block b;
    if (!L) {
        return b;
    }
    lua_getglobal(L, "GenerateBlock");
    if (!lua_isfunction(L,-1)) {
        std::cerr << "GenerateBlock was not found!" << std::endl;
        return b;
    }
    lua_pushnumber(L,position.x);
    lua_pushnumber(L,position.y);
    lua_pushnumber(L,position.z);
    lua_pushnumber(L,blocksSinceSkyVisible);
    if (CheckLua(L,lua_pcall(L,4,1,0))) {
        b.type = (int)lua_tonumber(L, -1);
    }
    lua_pop(L,1);
    return b;
}

Chunk Generator::GenerateChunk(int32_t cX, int32_t cZ) {
    Chunk c = Chunk();
    for (uint8_t x = 0; x < CHUNK_WIDTH_X; x++) {
        for (uint8_t z = 0; z < CHUNK_WIDTH_X; z++) {
            int8_t blocksSinceSkyVisible = 0;
            for (int8_t y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                Int3 blockPos = Int3 {x,y,z};
                Int3 chunkPos = Int3 {cX,0,cZ};
                Int3 globalPos = LocalToGlobalPosition(chunkPos,blockPos);
                Block b = GenerateBlock(globalPos,blocksSinceSkyVisible);
                if (b.type > BLOCK_AIR) {
                    blocksSinceSkyVisible++;
                }
                
                c.blocks[GetBlockIndex(blockPos)] = b;
            }
        }   
    }
    CalculateChunkLight(&c);
    return c;
}

// --- Helper Functions ---
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

Int3 GetPointPositionInChunk(int64_t seed, Int3 position, float verticalScale) {
    // Use different seeds for x, y, z to ensure unique coordinates
    int32_t randomized = SpatialPrng(seed, position);

    int32_t x = ((randomized ^ position.x) % CHUNK_WIDTH_X + CHUNK_WIDTH_X) % CHUNK_WIDTH_X;
    int32_t y = ((randomized ^ position.y) % CHUNK_HEIGHT  + CHUNK_HEIGHT ) % CHUNK_HEIGHT;
    y = y*verticalScale;
    int32_t z = ((randomized ^ position.z) % CHUNK_WIDTH_Z + CHUNK_WIDTH_Z) % CHUNK_WIDTH_Z;
    
    return Int3{x, y, z};
}

double FindDistanceToPoint(int64_t seed, Int3 position, float verticalScale) {
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
            
            Int3 goalBlockPos = GetPointPositionInChunk(seed, goalChunkPos, verticalScale);
            Int3 goalGlobalPos = LocalToGlobalPosition(goalChunkPos, goalBlockPos);
            
            double distance = GetDistance(position, goalGlobalPos);
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

double GetNoiseWorley(int64_t seed, Int3 position, double threshold, float verticalScale) {
    double distance = FindDistanceToPoint(seed, position, verticalScale);
    
    // Use smoothstep for more natural falloff
    return 1.0 - SmoothStep(0.0, threshold, distance);
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


// --- Lua Bindings Functions ---
int lua_Between(lua_State *L) {
    // Check if all arguments are numbers
    if (!CheckInt3(L)) {
        return 0;
    }

    int i = (int)lua_tonumber(L, 1);
    int a = (int)lua_tonumber(L, 2);
    int b = (int)lua_tonumber(L, 3);

    // Call Between and push the result
    bool result = Between(i, a, b);
    lua_pushboolean(L, result);

    return 1; // One return value on the Lua stack
}

int lua_SpatialPRNG(lua_State *L) {
    lua_getglobal(L, "seed"); // This sets the global 'seed' variable in Lua
    if (!lua_isnumber(L,1)) {
        std::cerr << "Invalid seed value!" << std::endl;
        return 0;
    }
    int64_t seed = (int64_t)lua_tonumber(L, 1);

    if (!CheckInt3(L)) {
        return 0;
    }

    int x = (int)lua_tonumber(L, 1);
    int y = (int)lua_tonumber(L, 2);
    int z = (int)lua_tonumber(L, 3);
    int result = SpatialPrng(seed,Int3{x,y,z});

    lua_pushnumber(L, result);
    return 1;
}

int lua_GetNoiseWorley(lua_State *L) {
    // Check for global 'seed'
    lua_getglobal(L, "seed");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Global 'seed' must be a number");
        return 0;
    }
    int64_t seed = (int64_t)lua_tonumber(L, -1);
    lua_pop(L, 1); // Pop 'seed'

    // Validate number of arguments
    if (lua_gettop(L) != 5) {
        luaL_error(L, "Expected exactly 5 arguments (x, y, z, threshold, verticalScale)");
        return 0;
    }

    // Validate and extract x, y, z
    if (!CheckInt3(L)) {
        return 0;
    }
    int x = (int)lua_tonumber(L, 1);
    int y = (int)lua_tonumber(L, 2);
    int z = (int)lua_tonumber(L, 3);
    Int3 position = Int3{x, y, z};

    // Validate and extract threshold
    if (!lua_isnumber(L, 4)) {
        luaL_error(L, "Threshold must be a numeric value");
        return 0;
    }
    double threshold = (double)lua_tonumber(L, 4);

    // Validate and extract verticalScale
    if (!lua_isnumber(L, 5)) {
        luaL_error(L, "Vertical scale must be a numeric value");
        return 0;
    }
    float verticalScale = (float)lua_tonumber(L, 5);

    // Call GetNoiseWorley and push result
    double result = GetNoiseWorley(seed, position, threshold, verticalScale);
    lua_pushnumber(L, result);
    return 1;
}

int lua_GetNaturalGrass(lua_State *L) {
    // Get the seed
    lua_getglobal(L, "seed");
    if (!lua_isnumber(L,1)) {
        std::cerr << "Invalid seed value!" << std::endl;
        return 0;
    }
    int64_t seed = (int64_t)lua_tonumber(L, 1);

    if (!CheckInt3(L)) {
        return 0;
    }
    int x = (int)lua_tonumber(L, 1);
    int y = (int)lua_tonumber(L, 2);
    int z = (int)lua_tonumber(L, 3);
    Int3 position = Int3{x, y, z};

    // Validate and extract threshold
    if (!lua_isnumber(L, 4)) {
        luaL_error(L, "Blocks since the Sky was visible must be a numeric value");
        return 0;
    }
    int bs = (int)lua_tonumber(L, 4);

    // Call the function
    Block result = GetNaturalGrass(seed, position, bs);
    lua_pushnumber(L, result.type);
    return 1;
}