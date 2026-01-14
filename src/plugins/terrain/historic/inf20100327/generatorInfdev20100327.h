#include "generator.h"


/**
 * @brief A faithful reimplementation of the Infdev 20100327 world generator
 * 
 */
class GeneratorInfdev20100327 : public Generator {
  private:
	JavaRandom rand;
	NoiseOctaves<NoisePerlin> noiseGen1;
	NoiseOctaves<NoisePerlin> noiseGen2;
	NoiseOctaves<NoisePerlin> noiseGen3;
	NoiseOctaves<NoisePerlin> noiseGen4;
	NoiseOctaves<NoisePerlin> noiseGen5;
	NoiseOctaves<NoisePerlin> noiseGen6;
	NoiseOctaves<NoisePerlin> mobSpawnerNoise;
	double InitializeNoiseField(double var1, double var3, double var5);
	bool WorldGenMinableGenerate(BlockType blockType, World *world, JavaRandom& rand, int32_t var3, int32_t var4, int32_t var5);
	bool GenerateMinable(BlockType blockType, World *world, JavaRandom& rand, int32_t var3, int32_t var4, int32_t var5);

  public:
	GeneratorInfdev20100327(int64_t seed, World *world);
	~GeneratorInfdev20100327() = default;
	std::shared_ptr<Chunk> GenerateChunk(Int2 chunkPos) override;
	bool PopulateChunk(Int2 chunkPos) override;
};