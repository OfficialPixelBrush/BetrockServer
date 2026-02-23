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
	const int32_t carveExtentLimit = 8;
	JavaRandom rand = JavaRandom();

  public:
	Beta173Caver();
	void CarveCavesForChunk(World *world, Int2 chunkPos, std::shared_ptr<Chunk> &c);
	void CarveCaves(Int2 chunkOffset, Int2 chunkPos, std::shared_ptr<Chunk> &c);
	void CarveCave(Int2 chunkPos, std::shared_ptr<Chunk> &c, Vec3 offset);
	void CarveCave(Int2 chunkPos, std::shared_ptr<Chunk> &c, Vec3 offset,
				   float tunnelRadius, float carveYaw, float carvePitch, int32_t tunnelStep, int32_t tunnelLength,
				   double verticalScale);
};