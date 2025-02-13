#pragma once
#include <lua.hpp>

#include "helper.h"
#include "blocks.h"
#include "config.h"
#include "lighting.h"
#include "luahelper.h"

class Generator {
    private:
        lua_State* L;
        int64_t seed;
        virtual Block GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible = 0);
    public:
        virtual Chunk GenerateChunk(int32_t cX, int32_t cZ);
        void PrepareGenerator(int64_t seed);
        ~Generator() {
            lua_close(L);
        }
};

// --- Helper Functions ---
int64_t Mix(int64_t a , int64_t b);
int32_t SpatialPrng(int64_t seed, Int3 position);
Int3 GetPointPositionInChunk(int64_t seed, Int3 position, Vec3 scale);
double FindDistanceToPoint(int64_t seed, Int3 position, Vec3 scale);
double SmoothStep(double edge0, double edge1, double x);
double GetNoiseWorley(Int3 position, double threshold, Vec3 scale);
Block GetNaturalGrass(int64_t seed, Int3 position, int32_t blocksSinceSkyVisible);
// --- Lua Bindings Functions ---
int lua_Between(lua_State *L);
int lua_SpatialPRNG(lua_State *L);
int lua_GetNoiseWorley(lua_State *L);
int lua_GetNaturalGrass(lua_State *L);