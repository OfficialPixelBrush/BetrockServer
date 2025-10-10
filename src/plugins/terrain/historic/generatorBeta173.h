#include "generator.h"

class GeneratorBeta173 : public Generator {
    private:
        std::unique_ptr<JavaRandom> rand;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen1;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen2;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen3;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen4;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen5;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen6;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> noiseGen7;
        std::unique_ptr<NoiseOctaves<NoisePerlin>> mobSpawnerNoise;
        void GenerateTerrain(int cX, int cY, std::unique_ptr<Chunk>& c, std::vector<double>& temperature);
        std::vector<double> GenerateTerrainNoise(std::vector<double> var1, int var2, int var3, int var4, int var5, int var6, int var7);
        std::vector<double> field_4224_q;
        std::vector<double> field_4229_d;
        std::vector<double> field_4228_e;
        std::vector<double> field_4227_f;
        std::vector<double> field_4226_g;
        std::vector<double> field_4225_h;
    public:
        GeneratorBeta173(int64_t seed, World* world);
        ~GeneratorBeta173() = default;
        std::unique_ptr<Chunk> GenerateChunk(int32_t cX, int32_t cZ) override;
        bool PopulateChunk(int32_t cX, int32_t cZ) override;
};