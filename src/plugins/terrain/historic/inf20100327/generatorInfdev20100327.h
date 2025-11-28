#include "generator.h"

// Implements Infdev 20100327 Generation very closely

class GeneratorInfdev20100327 : public Generator {
  private:
	std::unique_ptr<JavaRandom> rand;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen1;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen2;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen3;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen4;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen5;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen6;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> mobSpawnerNoise;
	double InitializeNoiseField(double var1, double var3, double var5);
	bool WorldGenMinableGenerate(int blockType, World *world, JavaRandom *rand, int var3, int var4, int var5);
	bool GenerateMinable(int blockType, World *world, JavaRandom *rand, int var3, int var4, int var5);

  public:
	GeneratorInfdev20100327(int64_t seed, World *world);
	~GeneratorInfdev20100327() = default;
	std::shared_ptr<Chunk> GenerateChunk(Int2 chunkPos) override;
	bool PopulateChunk(Int2 chunkPos) override;
};