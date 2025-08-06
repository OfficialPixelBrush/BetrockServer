#include "generator.h"

class GeneratorInfdev : public Generator {
    private:
        JavaRandom rand;
        std::unique_ptr<InfdevOctaves> noiseGen1;
        std::unique_ptr<InfdevOctaves> noiseGen2;
        std::unique_ptr<InfdevOctaves> noiseGen3;
        std::unique_ptr<InfdevOctaves> noiseGen4;
        std::unique_ptr<InfdevOctaves> noiseGen5;
        std::unique_ptr<InfdevOctaves> noiseGen6;
    public:
        GeneratorInfdev();
        ~GeneratorInfdev() = default;
        Chunk GenerateChunk(int32_t cX, int32_t cZ) override;
};