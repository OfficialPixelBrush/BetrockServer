#include "generator.h"
#include "biomes.h"

class GeneratorBeta173 : public Generator {
    private:
        std::unique_ptr<JavaRandom> rand;
        // Perlin Noise Generators
        std::unique_ptr<NoiseOctaves<NoisePerlin>> lowNoise;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> highNoise;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> selectorNoise;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> sandGravelNoise;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> stonePerlinNoise;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen1;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> depthNoise;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> mobSpawnerNoise;

        // Simplex Noise Generators
        std::unique_ptr<NoiseOctaves<NoiseSimplex>> temperatureNoise;
        std::unique_ptr<NoiseOctaves<NoiseSimplex>> humidityNoise;
        std::unique_ptr<NoiseOctaves<NoiseSimplex>> noiseSimplex1;

        // Stored noise Fields
        std::vector<double> terrainNoiseField;
        std::vector<double> lowNoiseField;
        std::vector<double> highNoiseField;
        std::vector<double> selectorNoiseField;
        std::vector<double> noiseField1;
        std::vector<double> depthNoiseField;

        std::vector<double> sandNoise;
        std::vector<double> gravelNoise;
        std::vector<double> stoneNoise;

        // Biome Vectors
        std::vector<Biome> biomeMap;
        std::vector<double> temperature;
        std::vector<double> humidity;
        std::vector<double> otherBiomeThing;

        void GenerateTerrain(int cX, int cZ, std::unique_ptr<Chunk>& c, std::vector<Biome> biomeMap, std::vector<double>& temperature);
        std::vector<double> GenerateTerrainNoise(std::vector<double> terrainMap, int x, int y, int z, int xMax, int yMax, int zMax);
        std::vector<Biome> GenerateBiomeMap(std::vector<Biome> biomeMap, int x, int z, int xMax, int zMax);
        void ReplaceBlocksForBiome(int cX, int cZ, std::unique_ptr<Chunk>& c, std::vector<Biome> biomeMap);
    public:
        GeneratorBeta173(int64_t seed, World* world);
        ~GeneratorBeta173() = default;
        std::unique_ptr<Chunk> GenerateChunk(int32_t cX, int32_t cZ) override;
        bool PopulateChunk(int32_t cX, int32_t cZ) override;
};