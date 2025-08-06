#include "generator.h"

// Implements Infdev 20100327 Generation very closely

class GeneratorInfdev20100327 : public Generator {
    private:
        JavaRandom rand;
        std::unique_ptr<InfdevOctaves> noiseGen1;
        std::unique_ptr<InfdevOctaves> noiseGen2;
        std::unique_ptr<InfdevOctaves> noiseGen3;
        std::unique_ptr<InfdevOctaves> noiseGen4;
        std::unique_ptr<InfdevOctaves> noiseGen5;
        std::unique_ptr<InfdevOctaves> noiseGen6;
        std::unique_ptr<InfdevOctaves> mobSpawnerNoise;
    public:
        GeneratorInfdev20100327();
        ~GeneratorInfdev20100327() = default;
        Chunk GenerateChunk(int32_t cX, int32_t cZ) override;
};