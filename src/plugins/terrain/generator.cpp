#include "generator.h"

// Prepare the Generator to utilize some preset numbers and functions
void Generator::PrepareGenerator(int64_t seed) {
	logger = &Betrock::Logger::Instance();
    this->seed = seed;
    L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushnumber(L, seed);
    lua_setglobal(L, "seed");

    lua_pushnumber(L, CHUNK_WIDTH_X-1);
    lua_setglobal(L, "CHUNK_WIDTH_X");
    lua_pushnumber(L, CHUNK_WIDTH_Z-1);
    lua_setglobal(L, "CHUNK_WIDTH_Z");
    lua_pushnumber(L, CHUNK_HEIGHT-1);
    lua_setglobal(L, "CHUNK_HEIGHT");

    // Add helper functions
    lua_register(L,"index", lua_Index);
    lua_register(L,"between", lua_Between);
    lua_register(L,"spatialPrng", lua_SpatialPRNG);
    lua_register(L,"getNoiseWorley", lua_GetNoiseWorley);
    lua_register(L,"getNoisePerlin2d", lua_GetNoisePerlin2D);
    lua_register(L,"getNoisePerlin3d", lua_GetNoisePerlin3D);
    lua_register(L,"getNaturalGrass", lua_GetNaturalGrass);
    
    // Execute a Lua script
    auto &cfg = Betrock::GlobalConfig::Instance();
    auto gen = cfg.Get("generator");
    if (luaL_dofile(L, (std::string("scripts/").append(gen)).c_str())) {
        lua_close(L);
        throw std::runtime_error(lua_tostring(L, -1));
    }

    // Load the plugins name
    lua_getglobal(L, "GenName");
    if (!lua_isstring(L,-1)) {
        logger->Warning("Invalid GenName!");
    } else {
        name = std::string(lua_tostring(L,-1));
    }

    // Load the plugins API version
    lua_getglobal(L, "GenApiVersion");
    if (!lua_isnumber(L,-1)) {
        lua_close(L);
        throw std::runtime_error("Invalid GenApiVersion!");
    } else {
        apiVersion = (int32_t)lua_tonumber(L,-1);
    }

    if (apiVersion > GENERATOR_LATEST_VERSION) {
        lua_close(L);
        throw std::runtime_error("\"" + name + "\" was made for a newer version of BetrockServer!");
    }
}

// Run the GenerateChunk function and pass its execution onto lua
// Then retrieve the generated Chunk data
Chunk Generator::GenerateChunk(int32_t cX, int32_t cZ) {
    Chunk c = Chunk();
    
    if (!L) {
        return c;
    }
    lua_getglobal(L, "GenerateChunk");
    if (!lua_isfunction(L,-1)) {
        throw std::runtime_error("GenerateChunk was not found!");
    }
    lua_pushnumber(L,cX);
    lua_pushnumber(L,cZ);
    if (CheckLua(L, lua_pcall(L, 2, 1, 0))) {
        if (lua_istable(L, -1)) {    
            for (int i = 1; i <= CHUNK_WIDTH_X*CHUNK_HEIGHT*CHUNK_WIDTH_Z; i++) {
                lua_rawgeti(L, -1, i);
                if (lua_istable(L, -1)) {
                    lua_rawgeti(L, -1, 1);
                    lua_rawgeti(L, -2, 2);
    
                    if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
                        c.blocks[i-1].type = lua_tointeger(L, -2);
                        c.blocks[i-1].meta = lua_tointeger(L, -1);
                    }
    
                    lua_pop(L, 2);  // Pop both numbers
                }
                lua_pop(L, 1);  // Pop table[i]
            }
    
            lua_pop(L, 1);  // Pop the table itself
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

Int3 GetPointPositionInChunk(int64_t seed, Int3 position, Vec3 scale) {
    // Use different seeds for x, y, z to ensure unique coordinates
    int32_t randomized = SpatialPrng(seed, position);

    int32_t x = ((randomized ^ position.x) % CHUNK_WIDTH_X + CHUNK_WIDTH_X) % CHUNK_WIDTH_X;
    int32_t y = ((randomized ^ position.y) % CHUNK_HEIGHT  + CHUNK_HEIGHT ) % CHUNK_HEIGHT;
    int32_t z = ((randomized ^ position.z) % CHUNK_WIDTH_Z + CHUNK_WIDTH_Z) % CHUNK_WIDTH_Z;
    x = x*scale.x;
    y = y*scale.y;
    z = x*scale.z;
    
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
    for (int cX = -1; cX < 2; cX++) {
        for (int cZ = -1; cZ < 2; cZ++) {
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

double GetNoisePerlin2D(int64_t seed, Vec3 position, int octaves) {
    return perlin.octave2D_01(position.x, position.z, octaves);
}

double GetNoisePerlin3D(int64_t seed, Vec3 position, int octaves) {
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

// --- Lua Bindings Functions ---
int lua_Index(lua_State *L) {
    // Check if all arguments are numbers
    if (!CheckNum3(L)) {
        return 0;
    }

    int x = (int)lua_tonumber(L, 1);
    int y = (int)lua_tonumber(L, 2);
    int z = (int)lua_tonumber(L, 3);
    if (x < 0) {
        x = 0;
    }
    if (x >= CHUNK_WIDTH_X) {
        x = CHUNK_WIDTH_X-1;
    }
    if (z < 0) {
        z = 0;
    }
    if (z >= CHUNK_WIDTH_Z) {
        z = CHUNK_WIDTH_Z-1;
    }
    if (y < 0) {
        y = 0;
    }
    if (y >= CHUNK_HEIGHT) {
        y = CHUNK_HEIGHT-1;
    }
    Int3 pos = Int3{x,y,z};

    // Call Between and push the result
    int32_t result = GetBlockIndex(pos)+1;
    lua_pushnumber(L, result);

    return 1; // One return value on the Lua stack
}

int lua_Between(lua_State *L) {
    // Check if all arguments are numbers
    if (!CheckNum3(L)) {
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

    if (!CheckNum3(L)) {
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
    if (lua_gettop(L) != 7) {
        luaL_error(L, "Expected exactly 5 arguments (x, y, z, threshold, scaleX, scaleY, scaleZ)");
        return 0;
    }

    // Validate and extract x, y, z
    if (!CheckNum3(L)) {
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
    if (!CheckNum3(L,5)) {
        luaL_error(L, "Scale must be a numeric values");
        return 0;
    }
    double scaleX = (float)lua_tonumber(L, 5);
    double scaleY = (float)lua_tonumber(L, 6);
    double scaleZ = (float)lua_tonumber(L, 7);
    Vec3 scale = {scaleX,scaleY,scaleZ};

    // Call GetNoiseWorley and push result
    double result = GetNoiseWorley(seed, position, threshold, scale);
    lua_pushnumber(L, result);
    return 1;
}

int lua_GetNoisePerlin2D(lua_State *L) {
    // Get the seed
    lua_getglobal(L, "seed");
    if (!lua_isnumber(L,1)) {
        std::cerr << "Invalid seed value!" << std::endl;
        return 0;
    }
    int64_t seed = (int64_t)lua_tonumber(L, 1);

    // Validate and extract x, y, z
    if (!CheckNum3(L)) {
        return 0;
    }
    double x = (double)lua_tonumber(L, 1);
    double y = (double)lua_tonumber(L, 2);
    double z = (double)lua_tonumber(L, 3);
    Vec3 position = Vec3{x, y, z};

    // Validate and extract octaves
    if (!lua_isnumber(L, 4)) {
        luaL_error(L, "Octaves must be a numeric value");
        return 0;
    }
    int octaves = (int)lua_tonumber(L, 4);

    // Call GetNoisePerlin2D and push result
    double result = GetNoisePerlin2D(seed, position, octaves);
    lua_pushnumber(L, result);
    return 1;
}

int lua_GetNoisePerlin3D(lua_State *L) {
    // Get the seed
    lua_getglobal(L, "seed");
    if (!lua_isnumber(L,1)) {
        std::cerr << "Invalid seed value!" << std::endl;
        return 0;
    }
    int64_t seed = (int64_t)lua_tonumber(L, 1);

    // Validate and extract x, y, z
    if (!CheckNum3(L)) {
        return 0;
    }
    double x = (double)lua_tonumber(L, 1);
    double y = (double)lua_tonumber(L, 2);
    double z = (double)lua_tonumber(L, 3);
    Vec3 position = Vec3{x, y, z};

    // Validate and extract octaves
    if (!lua_isnumber(L, 4)) {
        luaL_error(L, "Octaves must be a numeric value");
        return 0;
    }
    int octaves = (int)lua_tonumber(L, 4);

    // Call GetNoisePerlin3D and push result
    double result = GetNoisePerlin3D(seed, position, octaves);
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

    if (!CheckNum3(L)) {
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