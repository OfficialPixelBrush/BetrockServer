#pragma once
#include <lua.hpp>
#include <string>

#include "PerlinNoise.hpp"
#include "blocks.h"
#include "chunk.h"
#include "config.h"
#include "generatorVersionCheck.h"
#include "helper.h"
#include "javaRandom.h"
#include "luahelper.h"
#include "noiseOctaves.h"
#include "terrainhelper.h"
#include "world.h"

#include <cstdlib>
#include <ctime>

#define GENERATOR_DEFAULT_NAME "Generator"
#define GENERATOR_LATEST_VERSION 3

class Chunk;

class Generator {
  public:
	Generator(int64_t seed, World *world);
	virtual ~Generator();
	virtual std::shared_ptr<Chunk> GenerateChunk(Int2 chunkPos);
	virtual bool PopulateChunk(Int2 chunkPos);

  protected:
	Betrock::Logger *logger;
	int64_t seed;
	World *world;
};