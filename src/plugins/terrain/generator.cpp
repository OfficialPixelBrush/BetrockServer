#include "generator.h"

// Prepare the Generator to utilize some preset numbers and functions
void Generator::PrepareGenerator(int64_t seed, World* world) {
	logger = &Betrock::Logger::Instance();
    this->seed = seed;
    this->world = world;
    L = luaL_newstate();
    luaL_openlibs(L);

    lua_pushnumber(L, seed);
    lua_setglobal(L, "seed");

    lua_pushlightuserdata(L, this);
    lua_setglobal(L, "generator_instance");

    lua_pushnumber(L, CHUNK_WIDTH_X-1);
    lua_setglobal(L, "CHUNK_WIDTH_X");
    lua_pushnumber(L, CHUNK_WIDTH_Z-1);
    lua_setglobal(L, "CHUNK_WIDTH_Z");
    lua_pushnumber(L, CHUNK_HEIGHT-1);
    lua_setglobal(L, "CHUNK_HEIGHT");

    // Add helper functions
    lua_register(L,"index", Generator::lua_Index);
    lua_register(L,"between", Generator::lua_Between);
    lua_register(L,"spatialPrng", Generator::lua_SpatialPRNG);
    lua_register(L,"getNoiseWorley", Generator::lua_GetNoiseWorley);
    lua_register(L,"getNoisePerlin2d", Generator::lua_GetNoisePerlin2D);
    lua_register(L,"getNoisePerlin3d", Generator::lua_GetNoisePerlin3D);
    lua_register(L,"getNaturalGrass", Generator::lua_GetNaturalGrass);
    lua_register(L,"checkChunk", lua_CheckChunk);
    lua_register(L,"placeBlock", lua_PlaceBlock);
    lua_register(L,"getBlock", lua_GetBlock);
    
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

    rand = JavaRandom(this->seed);
    noiseGen1 = std::make_unique<InfdevOctaves>(rand, 16);
    noiseGen2 = std::make_unique<InfdevOctaves>(rand, 16);
    noiseGen3 = std::make_unique<InfdevOctaves>(rand, 8);
    noiseGen4 = std::make_unique<InfdevOctaves>(rand, 4);
    noiseGen5 = std::make_unique<InfdevOctaves>(rand, 4);
    noiseGen6 = std::make_unique<InfdevOctaves>(rand, 5);
}

Block Generator::DecodeBlock() {
    Block b;
    if (lua_istable(L, -1)) {
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);

        if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
            b.type = lua_tointeger(L, -2);
            b.meta = lua_tointeger(L, -1);
        }

        lua_pop(L, 2);  // Pop both numbers
    }
    return b;
}

// Implements Infdev 20100227-1433 Generation very closely
Chunk Generator::GenerateChunkInfdev(int32_t cX, int32_t cZ) {
    Chunk c = Chunk();
    int var3 = cX << 4;
    int var14 = cZ << 4;
    int var4 = 0;

    for(int var5 = var3; var5 < var3 + 16; ++var5) {
        for(int var6 = var14; var6 < var14 + 16; ++var6) {
            int var7 = var5 / 1024;
            int var8 = var6 / 1024;
            float var9 = (float)(this->noiseGen1.get()->generateNoise((double)((float)var5 / 0.03125F), 0.0D, (double)((float)var6 / 0.03125F)) - this->noiseGen2.get()->generateNoise((double)((float)var5 / 0.015625F), 0.0D, (double)((float)var6 / 0.015625F))) / 512.0F / 4.0F;
            float var10 = (float)this->noiseGen5.get()->generateNoise((double)((float)var5 / 4.0F), (double)((float)var6 / 4.0F));
            float var11 = (float)this->noiseGen6.get()->generateNoise((double)((float)var5 / 8.0F), (double)((float)var6 / 8.0F)) / 8.0F;
            var10 = var10 > 0.0F ? (float)(this->noiseGen3.get()->generateNoise((double)((float)var5 * 0.25714284F * 2.0F), (double)((float)var6 * 0.25714284F * 2.0F)) * (double)var11 / 4.0D) : (float)(this->noiseGen4.get()->generateNoise((double)((float)var5 * 0.25714284F), (double)((float)var6 * 0.25714284F)) * (double)var11);
            int var15 = (int)(var9 + 64.0F + var10);
            if((float)this->noiseGen5.get()->generateNoise((double)var5, (double)var6) < 0.0F) {
                var15 = var15 / 2 << 1;
                if((float)this->noiseGen5.get()->generateNoise((double)(var5 / 5), (double)(var6 / 5)) < 0.0F) {
                    ++var15;
                }
            }

            float value = static_cast<float>(std::rand()) / RAND_MAX;

            for(int var16 = 0; var16 < 128; ++var16) {
                int var17 = 0;
                if((var5 == 0 || var6 == 0) && var16 <= var15 + 2) {
                    var17 = BLOCK_OBSIDIAN;
                } else if(var16 == var15 + 1 && var15 >= 64 && value < 0.02D) {
                    var17 = BLOCK_DANDELION;
                } else if(var16 == var15 && var15 >= 64) {
                    var17 = BLOCK_GRASS;
                } else if(var16 <= var15 - 2) {
                    var17 = BLOCK_STONE;
                } else if(var16 <= var15) {
                    var17 = BLOCK_DIRT;
                } else if(var16 <= 64) {
                    var17 = BLOCK_WATER_STILL;
                }

                this->rand.setSeed((long)(var7 + var8 * 13871));
                int var12 = (var7 << 10) + 128 + this->rand.nextInt(512);
                int var13 = (var8 << 10) + 128 + this->rand.nextInt(512);
                var12 = var5 - var12;
                var13 = var6 - var13;
                if(var12 < 0) {
                    var12 = -var12;
                }

                if(var13 < 0) {
                    var13 = -var13;
                }

                if(var13 > var12) {
                    var12 = var13;
                }

                var12 = 127 - var12;
                if(var12 == 255) {
                    var12 = 1;
                }

                if(var12 < var15) {
                    var12 = var15;
                }

                if(var16 <= var12 && (var17 == 0 || var17 == BLOCK_WATER_STILL)) {
                    var17 = BLOCK_BRICKS;
                }

                if(var17 < 0) {
                    var17 = 0;
                }

                Block b;
                b.type = var17;
                c.blocks[var4++] = b;
            }
        }
    }
    // To prevent population
    c.populated = true;
    c.modified = true;
    return c;
}

// Run the GenerateChunk function and pass its execution onto lua
// Then retrieve the generated Chunk data
// This step is for ma
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
                c.blocks[i-1] = DecodeBlock();
                lua_pop(L, 1);  // Pop table[i]
            }
    
            lua_pop(L, 1);  // Pop the table itself
        }
    }
    // For initial loading a chunk needs to be marked as modified
    c.modified = true;
    return c;
}

bool Generator::PopulateChunk(int32_t cX, int32_t cZ) {
    if (apiVersion < API_GENERATOR_POPULATECHUNK) {
        return true;
    }
    if (!L) {
        return false;
    }
    lua_getglobal(L, "PopulateChunk");
    if (!lua_isfunction(L,-1)) {
        logger->Warning("PopulateChunk was not found! Skipping...");
        return true;
    }
    lua_pushnumber(L,cX);
    lua_pushnumber(L,cZ);
    CheckLua(L, lua_pcall(L, 2, 1, 0));
    return true;
}

// --- Lua Bindings Functions ---
int Generator::lua_Index(lua_State *L) {
    // Check if all arguments are numbers
    if (!CheckNum3(L)) {
        return 1;
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

int Generator::lua_Between(lua_State *L) {
    // Check if all arguments are numbers
    if (!CheckNum3(L)) {
        return 1;
    }

    int i = (int)lua_tonumber(L, 1);
    int a = (int)lua_tonumber(L, 2);
    int b = (int)lua_tonumber(L, 3);

    // Call Between and push the result
    bool result = Between(i, a, b);
    lua_pushboolean(L, result);

    return 1; // One return value on the Lua stack
}

int Generator::lua_SpatialPRNG(lua_State *L) {
    lua_getglobal(L, "seed"); // This sets the global 'seed' variable in Lua
    if (!lua_isnumber(L,1)) {
        std::cerr << "Invalid seed value!" << std::endl;
        return 1;
    }
    int64_t seed = (int64_t)lua_tonumber(L, 1);

    if (!CheckNum3(L)) {
        return 1;
    }

    int x = (int)lua_tonumber(L, 1);
    int y = (int)lua_tonumber(L, 2);
    int z = (int)lua_tonumber(L, 3);
    int result = SpatialPrng(seed,Int3{x,y,z});

    lua_pushnumber(L, result);
    return 1;
}

int Generator::lua_GetNoiseWorley(lua_State *L) {
    // Check for global 'seed'
    lua_getglobal(L, "seed");
    if (!lua_isnumber(L, -1)) {
        luaL_error(L, "Global 'seed' must be a number");
        return 1;
    }
    int64_t seed = (int64_t)lua_tonumber(L, -1);
    lua_pop(L, 1); // Pop 'seed'

    // Validate number of arguments
    if (lua_gettop(L) != 7) {
        luaL_error(L, "Expected exactly 5 arguments (x, y, z, threshold, scaleX, scaleY, scaleZ)");
        return 1;
    }

    // Validate and extract x, y, z
    if (!CheckNum3(L)) {
        return 1;
    }
    int x = (int)lua_tonumber(L, 1);
    int y = (int)lua_tonumber(L, 2);
    int z = (int)lua_tonumber(L, 3);
    Int3 position = Int3{x, y, z};

    // Validate and extract threshold
    if (!lua_isnumber(L, 4)) {
        luaL_error(L, "Threshold must be a numeric value");
        return 1;
    }
    double threshold = (double)lua_tonumber(L, 4);

    // Validate and extract verticalScale
    if (!CheckNum3(L,5)) {
        luaL_error(L, "Scale must be a numeric values");
        return 1;
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

int Generator::lua_GetNoisePerlin2D(lua_State *L) {
    // Get the seed
    lua_getglobal(L, "seed");
    if (!lua_isnumber(L,1)) {
        std::cerr << "Invalid seed value!" << std::endl;
        return 1;
    }
    int64_t seed = (int64_t)lua_tonumber(L, 1);

    // Validate and extract x, y, z
    if (!CheckNum3(L)) {
        return 1;
    }
    double x = (double)lua_tonumber(L, 1);
    double y = (double)lua_tonumber(L, 2);
    double z = (double)lua_tonumber(L, 3);
    Vec3 position = Vec3{x, y, z};

    // Validate and extract octaves
    if (!lua_isnumber(L, 4)) {
        luaL_error(L, "Octaves must be a numeric value");
        return 1;
    }
    int octaves = (int)lua_tonumber(L, 4);

    // Call GetNoisePerlin2D and push result
    double result = GetNoisePerlin2D(seed, position, octaves);
    lua_pushnumber(L, result);
    return 1;
}

int Generator::lua_GetNoisePerlin3D(lua_State *L) {
    // Get the seed
    lua_getglobal(L, "seed");
    if (!lua_isnumber(L,1)) {
        std::cerr << "Invalid seed value!" << std::endl;
        return 1;
    }
    int64_t seed = (int64_t)lua_tonumber(L, 1);

    // Validate and extract x, y, z
    if (!CheckNum3(L)) {
        return 1;
    }
    double x = (double)lua_tonumber(L, 1);
    double y = (double)lua_tonumber(L, 2);
    double z = (double)lua_tonumber(L, 3);
    Vec3 position = Vec3{x, y, z};

    // Validate and extract octaves
    if (!lua_isnumber(L, 4)) {
        luaL_error(L, "Octaves must be a numeric value");
        return 1;
    }
    int octaves = (int)lua_tonumber(L, 4);

    // Call GetNoisePerlin3D and push result
    double result = GetNoisePerlin3D(seed, position, octaves);
    lua_pushnumber(L, result);
    return 1;
}

int Generator::lua_GetNaturalGrass(lua_State *L) {
    // Get the seed
    lua_getglobal(L, "seed");
    if (!lua_isnumber(L,1)) {
        std::cerr << "Invalid seed value!" << std::endl;
        return 1;
    }
    int64_t seed = (int64_t)lua_tonumber(L, 1);

    if (!CheckNum3(L)) {
        return 1;
    }
    int x = (int)lua_tonumber(L, 1);
    int y = (int)lua_tonumber(L, 2);
    int z = (int)lua_tonumber(L, 3);
    Int3 position = Int3{x, y, z};

    // Validate and extract threshold
    if (!lua_isnumber(L, 4)) {
        luaL_error(L, "Blocks since the Sky was visible must be a numeric value");
        return 1;
    }
    int bs = (int)lua_tonumber(L, 4);

    // Call the function
    Block result = GetNaturalGrass(seed, position, bs);
    lua_pushnumber(L, result.type);
    return 1;
}

// Note: This is not needed for avoiding placing blocks in unloaded areas,
// since the world already checks if a chunk is actually there
int Generator::lua_CheckChunk(lua_State *L) {
    // Get the Generator* from Lua global
    lua_getglobal(L, "generator_instance");
    Generator* gen = static_cast<Generator*>(lua_touserdata(L, -1));
    lua_pop(L, 1); // remove from stack
    if (!gen) return luaL_error(L, "Generator instance not set");

    int x = (int)lua_tonumber(L, 1);
    int z = (int)lua_tonumber(L, 2);
    
    lua_pushboolean(L, gen->world->ChunkExists(x,z));
    return 1;
}

int Generator::lua_PlaceBlock(lua_State *L) {
    // Get the Generator* from Lua global
    lua_getglobal(L, "generator_instance");
    Generator* gen = static_cast<Generator*>(lua_touserdata(L, -1));
    lua_pop(L, 1); // remove from stack
    if (!gen) return luaL_error(L, "Generator instance not set");

    if (!CheckNum3(L)) {
        return 0;
    }
    int x = (int)lua_tonumber(L, 1);
    int y = (int)lua_tonumber(L, 2);
    int z = (int)lua_tonumber(L, 3);
    Int3 position = Int3{x, y, z};
    
    Block b = gen->DecodeBlock();
    
    gen->world->PlaceBlock(position,b.type,b.meta,false);
    return 0;
}

int Generator::lua_GetBlock(lua_State *L) {
    // Get the Generator* from Lua global
    lua_getglobal(L, "generator_instance");
    Generator* gen = static_cast<Generator*>(lua_touserdata(L, -1));
    lua_pop(L, 1); // remove from stack
    if (!gen) return luaL_error(L, "Generator instance not set");

    if (!CheckNum3(L)) {
        lua_pushnil(L);
        return 1;
    }
    int x = (int)lua_tonumber(L, 1);
    int y = (int)lua_tonumber(L, 2);
    int z = (int)lua_tonumber(L, 3);
    Int3 position = Int3{x, y, z};

    // Get the block at the given position
    Chunk* c = gen->world->GetChunk(x/16,z/16);
    if (!c) {
        lua_pushnil(L);
        return 1;
    }
    Block* b = gen->world->GetBlock(position);
    if (!b) {
        lua_pushnil(L);
        return 1;
    }

    // Create a table and push it to Lua stack
    lua_newtable(L);
    lua_pushnumber(L, b->type);
    lua_rawseti(L, -2, 1);
    lua_pushnumber(L, b->meta);
    lua_rawseti(L, -2, 2);

    return 1;
}