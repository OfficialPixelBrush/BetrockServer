#include "generator.h"

class GeneratorBeta173 : public Generator {
    private:
        std::unique_ptr<JavaRandom> rand;
        std::unique_ptr<InfdevOctaves> noiseGen1;
        std::unique_ptr<InfdevOctaves> noiseGen2;
        std::unique_ptr<InfdevOctaves> noiseGen3;
        std::unique_ptr<InfdevOctaves> noiseGen4;
        std::unique_ptr<InfdevOctaves> noiseGen5;
        std::unique_ptr<InfdevOctaves> noiseGen6;
        std::unique_ptr<InfdevOctaves> noiseGen7;
        std::unique_ptr<InfdevOctaves> mobSpawnerNoise;
        GenerateTerrain(int cX, int cY, std::unique_ptr<Chunk> c);
    public:
        GeneratorBeta173(int64_t seed, World* world);
        ~GeneratorBeta173() = default;
        std::unique_ptr<Chunk> GenerateChunk(int32_t cX, int32_t cZ) override;
        bool PopulateChunk(int32_t cX, int32_t cZ) override;
};