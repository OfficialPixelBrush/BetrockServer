#include "generator.h"

/**
 * @brief A faithful reimplementation of the Infdev 20100227 world generator (with seed support)
 * 
 */
class GeneratorInfdev20100227 : public Generator {
  private:
	std::unique_ptr<JavaRandom> rand;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen1;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen2;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen3;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen4;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen5;
	std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen6;

  public:
	GeneratorInfdev20100227(int64_t seed, World *world);
	~GeneratorInfdev20100227() = default;
	std::shared_ptr<Chunk> GenerateChunk(Int2 chunkPos) override;
	bool PopulateChunk(Int2 chunkPos) override;
};