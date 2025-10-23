#pragma once

#include <memory>
#include "../javaRandom.h"
#include "world.h"

class Beta173Caver {
    private:
        const int generatorOffset = 8;
        std::unique_ptr<JavaRandom> rand;
    public:
        Beta173Caver();
        void GenerateCavesForChunk(World* world, int cX, int cZ, std::unique_ptr<Chunk>& c);
        void GenerateCaves(int x, int z, int cX, int cZ, std::unique_ptr<Chunk>& c);
        void CarveCave(int cX, int cZ, std::unique_ptr<Chunk>& c, double xOffset, double yOffset, double zOffset);
        void CarveCave(int cX, int cZ, std::unique_ptr<Chunk>& c, double xOffset, double yOffset, double zOffset, float var10, float var11, float var12, int var13, int var14, double var15);
};