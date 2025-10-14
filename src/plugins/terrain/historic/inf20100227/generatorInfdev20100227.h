#include "generator.h"

// Implements Infdev 20100227-1433 Generation very closely

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
        GeneratorInfdev20100227(int64_t seed, World* world);
        ~GeneratorInfdev20100227() = default;
        std::unique_ptr<Chunk> GenerateChunk(int32_t cX, int32_t cZ) override;
        bool PopulateChunk(int32_t cX, int32_t cZ) override;
};