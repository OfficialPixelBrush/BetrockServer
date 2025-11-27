#include "generatorLua.h"

GeneratorLua::GeneratorLua(int64_t pSeed, World *pWorld) : Generator(pSeed, pWorld) {
	logger = &Betrock::Logger::Instance();
	seed = pSeed;
	world = pWorld;
	L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushnumber(L, seed);
	lua_setglobal(L, "seed");

	lua_pushlightuserdata(L, this);
	lua_setglobal(L, "generator_instance");

	lua_pushnumber(L, CHUNK_WIDTH_X - 1);
	lua_setglobal(L, "CHUNK_WIDTH_X");
	lua_pushnumber(L, CHUNK_WIDTH_Z - 1);
	lua_setglobal(L, "CHUNK_WIDTH_Z");
	lua_pushnumber(L, CHUNK_HEIGHT - 1);
	lua_setglobal(L, "CHUNK_HEIGHT");

	// Add helper functions
	lua_register(L, "index", GeneratorLua::lua_Index);
	lua_register(L, "between", GeneratorLua::lua_Between);
	lua_register(L, "spatialPrng", GeneratorLua::lua_SpatialPRNG);
	lua_register(L, "getNoiseWorley", GeneratorLua::lua_GetNoiseWorley);
	lua_register(L, "getNoisePerlin2d", GeneratorLua::lua_GetNoisePerlin2D);
	lua_register(L, "getNoisePerlin3d", GeneratorLua::lua_GetNoisePerlin3D);
	lua_register(L, "getNaturalGrass", GeneratorLua::lua_GetNaturalGrass);
	lua_register(L, "checkChunk", lua_CheckChunk);
	lua_register(L, "placeBlock", lua_PlaceBlock);
	lua_register(L, "getBlock", lua_GetBlock);

	// Execute a Lua script
	auto &cfg = Betrock::GlobalConfig::Instance();
	auto gen = cfg.Get("generator");
	if (luaL_dofile(L, (std::string("scripts/").append(gen)).c_str())) {
		lua_close(L);
		throw std::runtime_error(lua_tostring(L, -1));
	}

	// Load the plugins name
	lua_getglobal(L, "GenName");
	if (!lua_isstring(L, -1)) {
		logger->Warning("Invalid GenName!");
	} else {
		name = std::string(lua_tostring(L, -1));
	}

	// Load the plugins API version
	lua_getglobal(L, "GenApiVersion");
	if (!lua_isnumber(L, -1)) {
		lua_close(L);
		throw std::runtime_error("Invalid GenApiVersion!");
	} else {
		apiVersion = (int32_t)lua_tonumber(L, -1);
	}

	if (apiVersion > GENERATOR_LATEST_VERSION) {
		lua_close(L);
		throw std::runtime_error("\"" + name + "\" was made for a newer version of BetrockServer!");
	}
}

Block GeneratorLua::DecodeBlock() {
	Block b;
	if (lua_istable(L, -1)) {
		lua_rawgeti(L, -1, 1);
		lua_rawgeti(L, -2, 2);

		if (lua_isnumber(L, -2) && lua_isnumber(L, -1)) {
			b.type = lua_tointeger(L, -2);
			b.meta = lua_tointeger(L, -1);
		}

		lua_pop(L, 2); // Pop both numbers
	}
	return b;
}

// Run the GenerateChunk function and pass its execution onto lua
// Then retrieve the generated Chunk data
// This step is for ma
std::shared_ptr<Chunk> GeneratorLua::GenerateChunk(int32_t cX, int32_t cZ) {
	std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this->world, cX, cZ);

	if (!L) {
		return c;
	}
	lua_getglobal(L, "GenerateChunk");
	if (!lua_isfunction(L, -1)) {
		throw std::runtime_error("GenerateChunk was not found!");
	}
	lua_pushnumber(L, cX);
	lua_pushnumber(L, cZ);
	if (CheckLua(L, lua_pcall(L, 2, 1, 0))) {
		if (lua_istable(L, -1)) {
			for (int i = 1; i <= CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z; i++) {
				lua_rawgeti(L, -1, i);
				Block b = DecodeBlock();
				c->SetBlockTypeAndMeta(b.type, b.meta, BlockIndexToPosition(i - 1));
				lua_pop(L, 1); // Pop table[i]
			}

			lua_pop(L, 1); // Pop the table itself
		}
	}
	// For initial loading a chunk needs to be marked as modified
	c->GenerateHeightMap();
	c->state = ChunkState::Generated;
	c->modified = true;
	return c;
}

bool GeneratorLua::PopulateChunk(int32_t cX, int32_t cZ) {
	if (apiVersion < API_GENERATOR_POPULATECHUNK) {
		return true;
	}
	if (!L) {
		return false;
	}
	lua_getglobal(L, "PopulateChunk");
	if (!lua_isfunction(L, -1)) {
		logger->Warning("PopulateChunk was not found! Skipping...");
		return true;
	}
	lua_pushnumber(L, cX);
	lua_pushnumber(L, cZ);
	CheckLua(L, lua_pcall(L, 2, 1, 0));
	return true;
}

// --- Lua Bindings Functions ---
int GeneratorLua::lua_Index(lua_State *L) {
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
		x = CHUNK_WIDTH_X - 1;
	}
	if (z < 0) {
		z = 0;
	}
	if (z >= CHUNK_WIDTH_Z) {
		z = CHUNK_WIDTH_Z - 1;
	}
	if (y < 0) {
		y = 0;
	}
	if (y >= CHUNK_HEIGHT) {
		y = CHUNK_HEIGHT - 1;
	}

	// Call Between and push the result
	int32_t result = ((int32_t)(y + z * CHUNK_HEIGHT + (x * CHUNK_HEIGHT * CHUNK_WIDTH_Z))) + 1;
	lua_pushnumber(L, result);

	return 1; // One return value on the Lua stack
}

int GeneratorLua::lua_Between(lua_State *L) {
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

int GeneratorLua::lua_SpatialPRNG(lua_State *L) {
	lua_getglobal(L, "seed"); // This sets the global 'seed' variable in Lua
	if (!lua_isnumber(L, 1)) {
		std::cerr << "Invalid seed value!" << "\n";
		return 1;
	}
	int64_t seed = (int64_t)lua_tonumber(L, 1);

	if (!CheckNum3(L)) {
		return 1;
	}

	int x = (int)lua_tonumber(L, 1);
	int y = (int)lua_tonumber(L, 2);
	int z = (int)lua_tonumber(L, 3);
	int result = SpatialPrng(seed, Int3{x, y, z});

	lua_pushnumber(L, result);
	return 1;
}

int GeneratorLua::lua_GetNoiseWorley(lua_State *L) {
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
	if (!CheckNum3(L, 5)) {
		luaL_error(L, "Scale must be a numeric values");
		return 1;
	}
	double scaleX = lua_tonumber(L, 5);
	double scaleY = lua_tonumber(L, 6);
	double scaleZ = lua_tonumber(L, 7);
	Vec3 scale = {scaleX, scaleY, scaleZ};

	// Call GetNoiseWorley and push result
	double result = GetNoiseWorley(seed, position, threshold, scale);
	lua_pushnumber(L, result);
	return 1;
}

int GeneratorLua::lua_GetNoisePerlin2D(lua_State *L) {
	// Get the seed
	lua_getglobal(L, "seed");
	if (!lua_isnumber(L, 1)) {
		std::cerr << "Invalid seed value!" << "\n";
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

int GeneratorLua::lua_GetNoisePerlin3D(lua_State *L) {
	// Get the seed
	lua_getglobal(L, "seed");
	if (!lua_isnumber(L, 1)) {
		std::cerr << "Invalid seed value!" << "\n";
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

int GeneratorLua::lua_GetNaturalGrass(lua_State *L) {
	// Get the seed
	lua_getglobal(L, "seed");
	if (!lua_isnumber(L, 1)) {
		std::cerr << "Invalid seed value!" << "\n";
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
int GeneratorLua::lua_CheckChunk(lua_State *L) {
	// Get the GeneratorLua* from Lua global
	lua_getglobal(L, "generator_instance");
	GeneratorLua *gen = static_cast<GeneratorLua *>(lua_touserdata(L, -1));
	lua_pop(L, 1); // remove from stack
	if (!gen)
		return luaL_error(L, "GeneratorLua instance not set");

	int x = (int)lua_tonumber(L, 1);
	int z = (int)lua_tonumber(L, 2);

	lua_pushboolean(L, gen->world->ChunkExists(x, z));
	return 1;
}

int GeneratorLua::lua_PlaceBlock(lua_State *L) {
	// Get the GeneratorLua* from Lua global
	lua_getglobal(L, "generator_instance");
	GeneratorLua *gen = static_cast<GeneratorLua *>(lua_touserdata(L, -1));
	lua_pop(L, 1); // remove from stack
	if (!gen)
		return luaL_error(L, "GeneratorLua instance not set");

	if (!CheckNum3(L)) {
		return 0;
	}
	int x = (int)lua_tonumber(L, 1);
	int y = (int)lua_tonumber(L, 2);
	int z = (int)lua_tonumber(L, 3);
	Int3 position = Int3{x, y, z};

	Block b = gen->DecodeBlock();

	gen->world->PlaceBlock(position, b.type, b.meta);
	return 0;
}

int GeneratorLua::lua_GetBlock(lua_State *L) {
	// Get the GeneratorLua* from Lua global
	lua_getglobal(L, "generator_instance");
	GeneratorLua *gen = static_cast<GeneratorLua *>(lua_touserdata(L, -1));
	lua_pop(L, 1); // remove from stack
	if (!gen)
		return luaL_error(L, "GeneratorLua instance not set");

	if (!CheckNum3(L)) {
		lua_pushnil(L);
		return 1;
	}
	int x = (int)lua_tonumber(L, 1);
	int y = (int)lua_tonumber(L, 2);
	int z = (int)lua_tonumber(L, 3);
	Int3 position = Int3{x, y, z};

	// Create a table and push it to Lua stack
	lua_newtable(L);
	lua_pushnumber(L, gen->world->GetBlockType(position));
	lua_rawseti(L, -2, 1);
	lua_pushnumber(L, gen->world->GetBlockMeta(position));
	lua_rawseti(L, -2, 2);

	return 1;
}