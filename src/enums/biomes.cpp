#include "biomes.h"


uint8_t GetTopBlock(Biome biome) {
    if (biome == BIOME_DESERT) {
        return BLOCK_SAND;
    }
    return BLOCK_GRASS;
}

uint8_t GetFillerBlock(Biome biome) {
    if (biome == BIOME_ICEDESERT) {
        return BLOCK_SAND;
    }
    return BLOCK_DIRT;
}

Biome BiomeLUT[64*64];

Biome GetBiome(float temperature, float humidity) {
    humidity *= temperature;
    if (temperature < 0.1) {
        return BIOME_TUNDRA;
    }
    if (humidity < 0.2) {
        if (temperature < 0.5) {
            return BIOME_TUNDRA;
        }
        if (temperature < 0.95) {
            return BIOME_SAVANNA;
        }
        return BIOME_DESERT;
    }
    if (humidity > 0.5 && temperature < 0.7) {
        return BIOME_SWAMPLAND;
    }
    if (temperature < 0.5) {
        return BIOME_TAIGA;
    }
    if (temperature < 0.97) {
        if (humidity < 0.35) {
            return BIOME_SHRUBLAND;
        }
        return BIOME_FOREST;
    }
    if (humidity < 0.45) {
        return BIOME_PLAINS;
    }
    if (humidity < 0.9) {
        return BIOME_SEASONALFOREST;
    }
    return BIOME_RAINFOREST;
}

void GenerateBiomeLookup() {
    for(int temp = 0; temp < 64; ++temp) {
        for(int humi = 0; humi < 64; ++humi) {
            BiomeLUT[temp + humi * 64] = GetBiome((float)temp / 63.0F, (float)humi / 63.0F);
        }
    }
}

Biome GetBiomeFromLookup(float temperature, float humidity) {
    int temp = (int)(temperature * 63.0D);
    int humi = (int)(humidity * 63.0D);
    return BiomeLUT[temp + humi * 64];
}