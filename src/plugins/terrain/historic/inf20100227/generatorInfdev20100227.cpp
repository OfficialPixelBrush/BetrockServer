#include "generatorInfdev20100227.h"

GeneratorInfdev20100227::GeneratorInfdev20100227(int64_t seed, World* world) : Generator(seed, world) {
	logger = &Betrock::Logger::Instance();
    this->seed = seed;
    this->world = world;

    rand = std::make_unique<JavaRandom>(this->seed);
    noiseGen1 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    noiseGen2 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
    noiseGen3 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);
    noiseGen4 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
    noiseGen5 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
    noiseGen6 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 5);
}

std::shared_ptr<Chunk> GeneratorInfdev20100227::GenerateChunk(int32_t cX, int32_t cZ) {
    std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this->world,cX,cZ);
    int chunkStartX = cX << 4;
    int chunkStartZ = cZ << 4;
    int blockIndex = 0;

    for(int blockX = chunkStartX; blockX < chunkStartX + 16; ++blockX) {
        for(int blockZ = chunkStartZ; blockZ < chunkStartZ + 16; ++blockZ) {
            int regionX = blockX / 1024;
            int regionZ = blockZ / 1024;
            // Generate terrain height
            float noiseGen1Value = (float)(this->noiseGen1.get()->GenerateOctaves((double)((float)blockX / 0.03125F), 0.0D, (double)((float)blockZ / 0.03125F)) - this->noiseGen2.get()->GenerateOctaves((double)((float)blockX / 0.015625F), 0.0D, (double)((float)blockZ / 0.015625F))) / 512.0F / 4.0F;
            float noiseGen5Value = (float)this->noiseGen5.get()->GenerateOctaves((double)((float)blockX / 4.0F), (double)((float)blockZ / 4.0F));
            float noiseGen6Value = (float)this->noiseGen6.get()->GenerateOctaves((double)((float)blockX / 8.0F), (double)((float)blockZ / 8.0F)) / 8.0F;
            noiseGen5Value = noiseGen5Value > 0.0F ? (float)(this->noiseGen3.get()->GenerateOctaves((double)((float)blockX * 0.25714284F * 2.0F), (double)((float)blockZ * 0.25714284F * 2.0F)) * (double)noiseGen6Value / 4.0D) : (float)(this->noiseGen4.get()->GenerateOctaves((double)((float)blockX * 0.25714284F), (double)((float)blockZ * 0.25714284F)) * (double)noiseGen6Value);
            int terrainHeight = (int)(noiseGen1Value + 64.0F + noiseGen5Value);
            if((float)this->noiseGen5.get()->GenerateOctaves((double)blockX, (double)blockZ) < 0.0F) {
                terrainHeight = terrainHeight / 2 << 1;
                if((float)this->noiseGen5.get()->GenerateOctaves((double)(blockX / 5), (double)(blockZ / 5)) < 0.0F) {
                    ++terrainHeight;
                }
            }

            // Generate value for chunk decorations
            // TODO: Maybe replace this with java random for accuracy?
            float decorationChance = static_cast<float>(std::rand()) / RAND_MAX;

            for(int blockY = 0; blockY < CHUNK_HEIGHT; ++blockY) {
                // Determine Block Type based on parameters
                int blockType = BLOCK_AIR;
                if((blockX == 0 || blockZ == 0) && blockY <= terrainHeight + 2) {
                    blockType = BLOCK_OBSIDIAN;
                } else if(blockY == terrainHeight + 1 && terrainHeight >= WATER_LEVEL && decorationChance < 0.02f) {
                    blockType = BLOCK_DANDELION;
                } else if(blockY == terrainHeight && terrainHeight >= WATER_LEVEL) {
                    blockType = BLOCK_GRASS;
                } else if(blockY <= terrainHeight - 2) {
                    blockType = BLOCK_STONE;
                } else if(blockY <= terrainHeight) {
                    blockType = BLOCK_DIRT;
                } else if(blockY <= WATER_LEVEL) {
                    blockType = BLOCK_WATER_STILL;
                }

                // Generate Brick Pyramids
                this->rand->setSeed((long)(regionX + regionZ * 13871));
                int pyramidOffsetX = (regionX << 10) + CHUNK_HEIGHT + this->rand->nextInt(512);
                int pyramidOffsetZ = (regionZ << 10) + CHUNK_HEIGHT + this->rand->nextInt(512);
                pyramidOffsetX = blockX - pyramidOffsetX;
                pyramidOffsetZ = blockZ - pyramidOffsetZ;
                if(pyramidOffsetX < 0) pyramidOffsetX = -pyramidOffsetX;
                if(pyramidOffsetZ < 0) pyramidOffsetZ = -pyramidOffsetZ;
                if(pyramidOffsetZ > pyramidOffsetX) pyramidOffsetX = pyramidOffsetZ;
                pyramidOffsetX = (CHUNK_HEIGHT - 1) - pyramidOffsetX;
                if(pyramidOffsetX == 0xFF) pyramidOffsetX = 1;
                if(pyramidOffsetX < terrainHeight) pyramidOffsetX = terrainHeight;
                if(blockY <= pyramidOffsetX && (blockType == BLOCK_AIR || blockType == BLOCK_WATER_STILL)) {
                    blockType = BLOCK_BRICKS;
                }

                // Clamping
                if(blockType < BLOCK_AIR) blockType = BLOCK_AIR;

                c->SetBlockType(blockType, BlockIndexToPosition(blockIndex++));
            }
        }
    }
    // To prevent population
    c->GenerateHeightMap();
    c->state = ChunkState::Generated;
    c->modified = true;
    return c;
}

// Do nothing, since population didn't exist yet
bool GeneratorInfdev20100227::PopulateChunk(
    [[maybe_unused]] int32_t cX,
    [[maybe_unused]] int32_t cZ
) {
    return true;
}
