#pragma once
#include <lua.hpp>
#include <string>

#include "helper.h"
#include "blocks.h"
#include "config.h"
#include "luahelper.h"
#include "terrainhelper.h"
#include "world.h"
#include "generatorVersionCheck.h"
#include "historic/javaRandom.h"
#include "historic/infdevoctaves.h"
#include "lighting.h"
#include "chunk.h"
#include "PerlinNoise.hpp"

#include <cstdlib>
#include <ctime>

#define GENERATOR_DEFAULT_NAME "Generator"
#define GENERATOR_LATEST_VERSION 3

class Chunk;

class Generator {
    public:
        virtual std::unique_ptr<Chunk> GenerateChunk(int32_t cX, int32_t cZ);
        virtual bool PopulateChunk(int32_t cX, int32_t cZ);
        Generator(int64_t seed, World* world);
        virtual ~Generator() {
            if (L) {
                lua_close(L);
            }
        }
    protected:
        Betrock::Logger* logger;
        int64_t seed;
        World* world;
    private:
        std::string name = GENERATOR_DEFAULT_NAME;
        int32_t apiVersion = GENERATOR_LATEST_VERSION;
        lua_State* L;
        
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