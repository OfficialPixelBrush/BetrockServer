#pragma once
#include "biomes.h"
#include "noiseOctaves.h"
#include "noiseSimplex.h"

/**
 * @brief A faithful reimplementation of the Beta 1.7.3 biome generator
 * 
 */
class Beta173Biome {
    private:
        // Simplex Noise Generators
        NoiseOctaves<NoiseSimplex> temperatureNoiseGen;
        NoiseOctaves<NoiseSimplex> humidityNoiseGen;
        NoiseOctaves<NoiseSimplex> weirdnessNoiseGen;
    public:
        Beta173Biome(int64_t seed);
        void GenerateBiomeMap(std::vector<Biome>& biomeMap, std::vector<double>& temperature, std::vector<double>& humidity, std::vector<double>& weirdness, Int2 blockPos, Int2 max);
	    void GenerateTemperature(std::vector<double>& temperature, std::vector<double>& weirdness, Int2 chunkPos, Int2 max);
};