#pragma once

#include "../javaRandom.h"
#include "world.h"
#include <memory>

class Beta173Caver {
  private:
	const int carveExtentLimit = 8;
	std::unique_ptr<JavaRandom> rand;

  public:
	Beta173Caver();
	void GenerateCavesForChunk(World *world, int cX, int cZ, std::shared_ptr<Chunk> &c);
	void GenerateCaves(int cXoffset, int cZoffset, int cX, int cZ, std::shared_ptr<Chunk> &c);
	void CarveCave(int cX, int cZ, std::shared_ptr<Chunk> &c, double xOffset, double yOffset, double zOffset);
	void CarveCave(int cX, int cZ, std::shared_ptr<Chunk> &c, double xOffset, double yOffset, double zOffset,
				   float tunnelRadius, float carveYaw, float carvePitch, int tunnelStep, int tunnelLength,
				   double verticalScale);
};