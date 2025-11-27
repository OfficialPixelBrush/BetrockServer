#pragma once

#include "javaRandom.h"
#include "world.h"

class Beta173Feature {
  private:
	int16_t id = BLOCK_AIR;
	int8_t meta = 0;

  public:
	Beta173Feature() {};
	Beta173Feature(int16_t id);
	Beta173Feature(int16_t id, int8_t meta);
	bool GenerateLake(World *world, JavaRandom *rand, int x, int y, int z);
	bool GenerateDungeon(World *world, JavaRandom *rand, int x, int y, int z);
	Item GenerateDungeonChestLoot(JavaRandom *rand);
	std::string PickMobToSpawn(JavaRandom *rand);
	bool GenerateClay(World *world, JavaRandom *rand, int x, int y, int z, int numberOfBlocks = 0);
	bool GenerateMinable(World *world, JavaRandom *rand, int x, int y, int z, int numberOfBlocks = 0);
	bool GenerateFlowers(World *world, JavaRandom *rand, int x, int y, int z);
	bool GenerateTallgrass(World *world, JavaRandom *rand, int x, int y, int z);
	bool GenerateDeadbush(World *world, JavaRandom *rand, int x, int y, int z);
	bool GenerateSugarcane(World *world, JavaRandom *rand, int x, int y, int z);
	bool GeneratePumpkins(World *world, JavaRandom *rand, int x, int y, int z);
	bool GenerateCacti(World *world, JavaRandom *rand, int x, int y, int z);
	bool GenerateLiquid(World *world, JavaRandom *rand, int x, int y, int z);
};