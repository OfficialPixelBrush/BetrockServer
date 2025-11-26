#include "generator.h"
#include "biomes.h"
#include "beta173Caver.h"
#include "beta173Feature.h"
#include "beta173Tree.h"

class GeneratorBeta173 : public Generator {
    private:
        std::unique_ptr<JavaRandom> rand;
        // Perlin Noise Generators
        std::unique_ptr<NoiseOctaves<NoisePerlin>> lowNoiseGen;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> highNoiseGen;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> selectorNoiseGen;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> sandGravelNoiseGen;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> stoneNoiseGen;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> continentalnessNoiseGen;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> depthNoiseGen;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> treeDensityNoiseGen;

        // Simplex Noise Generators
        std::unique_ptr<NoiseOctaves<NoiseSimplex>> temperatureNoiseGen;
        std::unique_ptr<NoiseOctaves<NoiseSimplex>> humidityNoiseGen;
        std::unique_ptr<NoiseOctaves<NoiseSimplex>> weirdnessNoiseGen;

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

        // Biome Vectors
        std::vector<Biome> biomeMap;
        std::vector<double> temperature;
        std::vector<double> humidity;
        std::vector<double> weirdness;

        // Cave Gen
        std::unique_ptr<Beta173Caver> caver;

        void GenerateTerrain(int cX, int cZ, std::shared_ptr<Chunk>& c);
        void GenerateTerrainNoise(std::vector<double>& terrainMap, int x, int y, int z, int xMax, int yMax, int zMax);
        void GenerateBiomeMap(int x, int z, int xMax, int zMax);
        void GenerateTemperature(int x, int z, int xMax, int zMax);
        void ReplaceBlocksForBiome(int cX, int cZ, std::shared_ptr<Chunk>& c);
        Biome GetBiomeAt(int worldX, int worldZ);
    public:
        GeneratorBeta173(int64_t seed, World* world);
        ~GeneratorBeta173() = default;
        std::shared_ptr<Chunk> GenerateChunk(int32_t cX, int32_t cZ) override;
        bool PopulateChunk(int32_t cX, int32_t cZ) override;
};