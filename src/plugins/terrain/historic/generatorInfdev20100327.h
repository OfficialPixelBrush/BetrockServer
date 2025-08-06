#include "generator.h"

// Implements Infdev 20100327 Generation very closely

class GeneratorInfdev20100327 : public Generator {
    private:
        std::unique_ptr<JavaRandom> rand;
        std::unique_ptr<InfdevOctaves> noiseGen1;
        std::unique_ptr<InfdevOctaves> noiseGen2;
        std::unique_ptr<InfdevOctaves> noiseGen3;
        //std::unique_ptr<InfdevOctaves> noiseGen4;
        //std::unique_ptr<InfdevOctaves> noiseGen5;
        //std::unique_ptr<InfdevOctaves> noiseGen6;
        std::unique_ptr<InfdevOctaves> mobSpawnerNoise;
        double InitializeNoiseField(double var1, double var3, double var5);
        bool GenerateMinable(int blockType, World* world, JavaRandom* rand, int var3, int var4, int var5);
    public:
        GeneratorInfdev20100327(int64_t seed, World* world);
        ~GeneratorInfdev20100327() = default;
        Chunk GenerateChunk(int32_t cX, int32_t cZ) override;
        bool PopulateChunk(int32_t cX, int32_t cZ) override;
};