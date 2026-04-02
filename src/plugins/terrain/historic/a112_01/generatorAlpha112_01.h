#include "../b173/beta173Caver.h"
//#include "beta173Feature.h"
//#include "beta173Tree.h"
#include "generator.h"
#include "blockHelper.h"

/**
 * @brief A faithful reimplementation of the Beta 1.7.3 world generator
 * 
 */
class GeneratorAlpha112_01 : public Generator {
  private:
	JavaRandom rand;
	// Perlin Noise Generators
	NoiseOctaves<NoisePerlin> lowNoiseGen;
	NoiseOctaves<NoisePerlin> highNoiseGen;
	NoiseOctaves<NoisePerlin> selectorNoiseGen;
	NoiseOctaves<NoisePerlin> sandGravelNoiseGen;
	NoiseOctaves<NoisePerlin> stoneNoiseGen;
	NoiseOctaves<NoisePerlin> continentalnessNoiseGen;
	NoiseOctaves<NoisePerlin> depthNoiseGen;
	NoiseOctaves<NoisePerlin> treeDensityNoiseGen;

	// Stored noise Fields
	std::vector<double> terrainNoiseField;
	std::vector<double> lowNoiseField;
	std::vector<double> highNoiseField;
	std::vector<double> selectorNoiseField;
	std::vector<double> continentalnessNoiseField;
	std::vector<double> depthNoiseField;

	std::vector<double> sandNoise;
	std::vector<double> gravelNoise;
	std::vector<double> stoneNoise;

	// Cave Gen
	std::unique_ptr<Beta173Caver> caver;

	void GenerateTerrain(Int2 chunkPos, std::shared_ptr<Chunk> &c);
	void GenerateTerrainNoise(std::vector<double> &terrainMap, Int3 chunkPos, Int3 max);
	void ReplaceSurfaceBlocks(Int2 chunkPos, std::shared_ptr<Chunk> &c);

  public:
	GeneratorAlpha112_01(int64_t seed, World *world);
	~GeneratorAlpha112_01() = default;
	std::shared_ptr<Chunk> GenerateChunk(Int2 chunkPos) override;
	bool PopulateChunk(Int2 chunkPos) override;
};