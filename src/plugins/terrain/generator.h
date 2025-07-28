#pragma once
#include <lua.hpp>
#include <string>

#include "helper.h"
#include "blocks.h"
#include "config.h"
#include "luahelper.h"
#include "world.h"
#include "generatorVersionCheck.h"

#define GENERATOR_DEFAULT_NAME "Generator"
#define GENERATOR_LATEST_VERSION 3

class Generator {
    public:
        virtual Chunk GenerateChunk(int32_t cX, int32_t cZ);
        virtual bool PopulateChunk(int32_t cX, int32_t cZ);
        void PrepareGenerator(int64_t seed, World* world);
        ~Generator() {
            if (L) {
                lua_close(L);
            }
        }
    private:
        Betrock::Logger* logger;
        std::string name = GENERATOR_DEFAULT_NAME;
        int32_t apiVersion = GENERATOR_LATEST_VERSION;
        lua_State* L;
        int64_t seed;
        World* world;
        
        Block DecodeBlock();
        
        // --- TERRAIN GEN RELATED ---
        const siv::PerlinNoise::seed_type seedp = 0;
        const siv::PerlinNoise perlin{ seedp };

        static int lua_Index(lua_State *L);
        static int lua_Between(lua_State *L);
        static int lua_SpatialPRNG(lua_State *L);
        static int lua_GetNoiseWorley(lua_State *L);
        static int lua_GetNoisePerlin2D(lua_State *L);
        static int lua_GetNoisePerlin3D(lua_State *L);
        static int lua_GetNaturalGrass(lua_State *L);
        static int lua_CheckChunk(lua_State *L);
        static int lua_PlaceBlock(lua_State *L);
        static int lua_GetBlock(lua_State *L);
};