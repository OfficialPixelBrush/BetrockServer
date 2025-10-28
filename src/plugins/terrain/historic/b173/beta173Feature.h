#pragma once

#include "world.h"
#include "javaRandom.h"

class Beta173Feature {
    private:
        int16_t id = BLOCK_AIR;
    public:
        Beta173Feature() { };
        Beta173Feature(int16_t id);
        bool GenerateLake(World* world, JavaRandom* rand, int x, int y, int z);
        bool GenerateDungeon(World* world, JavaRandom* rand, int x, int y, int z);
        Item GenerateDungeonChestLoot(JavaRandom* rand);
        std::string PickMobToSpawn(JavaRandom* rand);
        bool GenerateClay(World* world, JavaRandom* rand, int x, int y, int z, int numberOfBlocks = 0);
        bool GenerateMinable(World* world, JavaRandom* rand, int x, int y, int z, int numberOfBlocks = 0);
};