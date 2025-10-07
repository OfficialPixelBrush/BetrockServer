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
        Generator(int64_t seed, World* world);
        virtual ~Generator();
        virtual std::unique_ptr<Chunk> GenerateChunk(int32_t cX, int32_t cZ);
        virtual bool PopulateChunk(int32_t cX, int32_t cZ);
    protected:
        Betrock::Logger* logger;
        int64_t seed;
        World* world;
};