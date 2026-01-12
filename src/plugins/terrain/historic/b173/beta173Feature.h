#pragma once

#include "javaRandom.h"
#include "world.h"
#include "blockHelper.h"

/**
 * @brief Beta 1.7.3 Feature Generators
 * This class wraps up all the features that Beta 1.7.3 can generate
 * into a single class for ease of access.
 */
class Beta173Feature {
  private:
	BlockType type = BLOCK_AIR;
	int8_t meta = 0;

  public:
	Beta173Feature() {};
	Beta173Feature(BlockType type);
	Beta173Feature(BlockType type, int8_t meta);
	bool GenerateLake(World *world, JavaRandom *rand, Int3 pos);
	bool GenerateDungeon(World *world, JavaRandom *rand, Int3 pos);
	Item GenerateDungeonChestLoot(JavaRandom *rand);
	std::string PickMobToSpawn(JavaRandom *rand);
	bool GenerateClay(World *world, JavaRandom *rand, Int3 pos, int32_t numberOfBlocks = 0);
	bool GenerateMinable(World *world, JavaRandom *rand, Int3 pos, int32_t numberOfBlocks = 0);
	bool GenerateFlowers(World *world, JavaRandom *rand, Int3 pos);
	bool GenerateTallgrass(World *world, JavaRandom *rand, Int3 pos);
	bool GenerateDeadbush(World *world, JavaRandom *rand, Int3 pos);
	bool GenerateSugarcane(World *world, JavaRandom *rand, Int3 pos);
	bool GeneratePumpkins(World *world, JavaRandom *rand, Int3 pos);
	bool GenerateCacti(World *world, JavaRandom *rand, Int3 pos);
	bool GenerateLiquid(World *world, JavaRandom *rand, Int3 pos);
};