#include "generatorInfdev20100227.h"

GeneratorInfdev20100227::GeneratorInfdev20100227(int64_t pSeed, World *pWorld) : Generator(pSeed, pWorld) {
	logger = &Betrock::Logger::Instance();
	this->seed = pSeed;
	this->world = pWorld;

	rand = std::make_unique<JavaRandom>(this->seed);
	noiseGen1 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
	noiseGen2 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 16);
	noiseGen3 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 8);
	noiseGen4 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
	noiseGen5 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 4);
	noiseGen6 = std::make_unique<NoiseOctaves<NoisePerlin>>(rand.get(), 5);
}

std::shared_ptr<Chunk> GeneratorInfdev20100227::GenerateChunk(Int2 chunkPos) {
	std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this->world, chunkPos);
	c->state = ChunkState::Generating;
	int32_t chunkStartX = chunkPos.x << 4;
	int32_t chunkStartZ = chunkPos.y << 4;
	int32_t blockIndex = 0;

	for (int32_t blockX = chunkStartX; blockX < chunkStartX + 16; ++blockX) {
		for (int32_t blockZ = chunkStartZ; blockZ < chunkStartZ + 16; ++blockZ) {
			int32_t regionX = blockX / 1024;
			int32_t regionZ = blockZ / 1024;
			// Generate terrain height
			float noiseGen1Value =
				(float)(this->noiseGen1.get()->GenerateOctaves((double)((float)blockX / 0.03125F), 0.0,
															   (double)((float)blockZ / 0.03125F)) -
						this->noiseGen2.get()->GenerateOctaves((double)((float)blockX / 0.015625F), 0.0,
															   (double)((float)blockZ / 0.015625F))) /
				512.0F / 4.0F;
			float noiseGen5Value = (float)this->noiseGen5.get()->GenerateOctaves((double)((float)blockX / 4.0F),
																				 (double)((float)blockZ / 4.0F));
			float noiseGen6Value = (float)this->noiseGen6.get()->GenerateOctaves((double)((float)blockX / 8.0F),
																				 (double)((float)blockZ / 8.0F)) /
								   8.0F;
			noiseGen5Value =
				noiseGen5Value > 0.0F
					? (float)(this->noiseGen3.get()->GenerateOctaves((double)((float)blockX * 0.25714284F * 2.0F),
																	 (double)((float)blockZ * 0.25714284F * 2.0F)) *
							  (double)noiseGen6Value / 4.0)
					: (float)(this->noiseGen4.get()->GenerateOctaves((double)((float)blockX * 0.25714284F),
																	 (double)((float)blockZ * 0.25714284F)) *
							  (double)noiseGen6Value);
			int32_t terrainHeight = (int)(noiseGen1Value + 64.0F + noiseGen5Value);
			if ((float)this->noiseGen5.get()->GenerateOctaves((double)blockX, (double)blockZ) < 0.0F) {
				terrainHeight = terrainHeight / 2 << 1;
				if ((float)this->noiseGen5.get()->GenerateOctaves((double)(blockX / 5), (double)(blockZ / 5)) < 0.0F) {
					++terrainHeight;
				}
			}

			// Generate value for chunk decorations
			// TODO: Maybe replace this with java random for accuracy?
			float decorationChance = static_cast<float>(std::rand()) / RAND_MAX;

			for (int32_t blockY = 0; blockY < CHUNK_HEIGHT; ++blockY) {
				// Determine Block Type based on parameters
				BlockType blockType = BLOCK_AIR;
				if ((blockX == 0 || blockZ == 0) && blockY <= terrainHeight + 2) {
					blockType = BLOCK_OBSIDIAN;
				} else if (blockY == terrainHeight + 1 && terrainHeight >= WATER_LEVEL && decorationChance < 0.02f) {
					blockType = BLOCK_DANDELION;
				} else if (blockY == terrainHeight && terrainHeight >= WATER_LEVEL) {
					blockType = BLOCK_GRASS;
				} else if (blockY <= terrainHeight - 2) {
					blockType = BLOCK_STONE;
				} else if (blockY <= terrainHeight) {
					blockType = BLOCK_DIRT;
				} else if (blockY <= WATER_LEVEL) {
					blockType = BLOCK_WATER_STILL;
				}

				// Generate Brick Pyramids
				this->rand->setSeed((long)(regionX + regionZ * 13871));
				int32_t pyramidOffsetX = (regionX << 10) + CHUNK_HEIGHT + this->rand->nextInt(512);
				int32_t pyramidOffsetZ = (regionZ << 10) + CHUNK_HEIGHT + this->rand->nextInt(512);
				pyramidOffsetX = blockX - pyramidOffsetX;
				pyramidOffsetZ = blockZ - pyramidOffsetZ;
				if (pyramidOffsetX < 0)
					pyramidOffsetX = -pyramidOffsetX;
				if (pyramidOffsetZ < 0)
					pyramidOffsetZ = -pyramidOffsetZ;
				if (pyramidOffsetZ > pyramidOffsetX)
					pyramidOffsetX = pyramidOffsetZ;
				pyramidOffsetX = (CHUNK_HEIGHT - 1) - pyramidOffsetX;
				if (pyramidOffsetX == 0xFF)
					pyramidOffsetX = 1;
				if (pyramidOffsetX < terrainHeight)
					pyramidOffsetX = terrainHeight;
				if (blockY <= pyramidOffsetX && (blockType == BLOCK_AIR || blockType == BLOCK_WATER_STILL)) {
					blockType = BLOCK_BRICKS;
				}

				// Clamping
				if (blockType < BLOCK_AIR)
					blockType = BLOCK_AIR;

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
bool GeneratorInfdev20100227::PopulateChunk([[maybe_unused]] Int2 chunkPos) { return true; }
