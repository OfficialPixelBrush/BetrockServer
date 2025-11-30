#pragma once

#include "../javaRandom.h"
#include "world.h"
#include <memory>

/**
 * @brief Used to carve caves into the world
 * 
 */
class Beta173Caver {
  private:
	const int carveExtentLimit = 8;
	std::unique_ptr<JavaRandom> rand;

  public:
	Beta173Caver();
	void GenerateCavesForChunk(World *world, Int2 chunkPos, std::shared_ptr<Chunk> &c);
	void GenerateCaves(Int2 chunkOffset, Int2 chunkPos, std::shared_ptr<Chunk> &c);
	void CarveCave(Int2 chunkPos, std::shared_ptr<Chunk> &c, Vec3 offset);
	void CarveCave(Int2 chunkPos, std::shared_ptr<Chunk> &c, Vec3 offset,
				   float tunnelRadius, float carveYaw, float carvePitch, int tunnelStep, int tunnelLength,
				   double verticalScale);
};