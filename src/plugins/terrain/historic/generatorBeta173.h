#include "generator.h"
#include "biomes.h"

class GeneratorBeta173 : public Generator {
    private:
        std::unique_ptr<JavaRandom> rand;
        // Perlin Noise Generators
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen1;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen2;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen3;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen4;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen5;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen6;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen7;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> mobSpawnerNoise;

        // Simplex Noise Generators
        std::unique_ptr<NoiseOctaves<NoiseSimplex>> noiseSimplex1;
        std::unique_ptr<NoiseOctaves<NoiseSimplex>> noiseSimplex2;
        std::unique_ptr<NoiseOctaves<NoiseSimplex>> noiseSimplex3;

        // Stored noise
        std::vector<double> terrainNoise;
        std::vector<double> noiseField1;
        std::vector<double> noiseField2;
        std::vector<double> noiseField3;
        std::vector<double> noiseField4;
        std::vector<double> noiseField5;

        // Biome Vectors
        std::vector<Biome> biomeMap;
        std::vector<double> temperature;
        std::vector<double> humidity;
        std::vector<double> otherBiomeThing;

        void GenerateTerrain(int cX, int cY, std::unique_ptr<Chunk>& c, std::vector<Biome> biomeMap, std::vector<double>& temperature);
        std::vector<double> GenerateTerrainNoise(std::vector<double> terrainMap, int x, int y, int z, int xMax, int yMax, int zMax);
        std::vector<Biome> GenerateBiomeMap(std::vector<Biome> biomeMap, int x, int z, int xMax, int zMax);
    public:
        GeneratorBeta173(int64_t seed, World* world);
        ~GeneratorBeta173() = default;
        std::unique_ptr<Chunk> GenerateChunk(int32_t cX, int32_t cZ) override;
        bool PopulateChunk(int32_t cX, int32_t cZ) override;
};