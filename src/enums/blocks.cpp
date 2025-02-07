#include "blocks.h"

bool IsTranslucent(int16_t id) {
    if (id == BLOCK_WATER_FLOWING ||
        id == BLOCK_WATER_STILL ||
        id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_LEAVES ||
        id == BLOCK_GLASS ||
        id == BLOCK_ICE ||
        id == BLOCK_MOB_SPAWNER)
    {
        return true;
    }
    return false;
}

uint8_t GetTranslucency(int16_t id, uint8_t &skylight) {
    uint8_t subtractor = 0;
    // Water seems to drop the skylight brightness by 3 levels with each block
    if (id == BLOCK_WATER_FLOWING ||
        id == BLOCK_WATER_STILL ||
        id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL) {
        subtractor = 3;
    }

    // Meanwhile, leaves seem to drop the skylight by 1 with each block, until reaching 12
    if (id == BLOCK_LEAVES && skylight > 12) {
        subtractor = 1;
    }

    if (
        id == BLOCK_GLASS ||
        id == BLOCK_ICE ||
        id == BLOCK_MOB_SPAWNER
    ) {
        subtractor = 1;
    }

    // Do this check to prevent a potential overflow
    if (skylight < subtractor) {
        skylight = 0;
    } else {
        skylight -= subtractor;
    }
    return skylight;
}

bool IsTransparent(int16_t id) {
    if (id == BLOCK_AIR ||
        id == BLOCK_SAPLING ||
        id == BLOCK_RAIL_POWERED ||
        id == BLOCK_RAIL_DETECTOR ||
        id == BLOCK_COBWEB ||
        id == BLOCK_TALLGRASS ||
        id == BLOCK_DEADBUSH ||
        id == BLOCK_DANDELION ||
        id == BLOCK_ROSE ||
        id == BLOCK_MUSHROOM_BROWN ||
        id == BLOCK_MUSHROOM_RED ||
        id == BLOCK_TORCH ||
        id == BLOCK_FIRE ||
        id == BLOCK_MOB_SPAWNER ||
        id == BLOCK_REDSTONE_WIRE ||
        id == BLOCK_CROP_WHEAT ||
        id == BLOCK_SIGN_STANDING ||
        id == BLOCK_LADDER ||
        id == BLOCK_RAIL ||
        id == BLOCK_SIGN_WALL ||
        id == BLOCK_LEVER ||
        id == BLOCK_PRESSURE_PLATE_STONE ||
        id == BLOCK_PRESSURE_PLATE_WOOD ||
        id == BLOCK_REDSTONE_TORCH_OFF ||
        id == BLOCK_REDSTONE_TORCH_ON ||
        id == BLOCK_BUTTON_STONE ||
        id == BLOCK_CACTUS ||
        id == BLOCK_SUGARCANE ||
        id == BLOCK_FENCE ||
        id == BLOCK_REDSTONE_REPEATER_OFF ||
        id == BLOCK_REDSTONE_REPEATER_ON
        )
    {
        return true;
    }
    return false;
}

// TODO: Turn into int8_t for different light levels
bool IsEmissive(int16_t id) {
    if (id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_TORCH ||
        id == BLOCK_FIRE ||
        id == BLOCK_FURNACE_LIT ||
        id == BLOCK_ORE_REDSTONE_GLOWING || 
        id == BLOCK_REDSTONE_TORCH_ON ||
        id == BLOCK_GLOWSTONE ||
        id == BLOCK_NETHER_PORTAL ||
        id == BLOCK_PUMPKIN_LIT)
    {
        return true;
    }
    return false;
}

uint8_t GetEmissiveness(int16_t id) {
    if (id == BLOCK_LAVA_FLOWING ||
        id == BLOCK_LAVA_STILL ||
        id == BLOCK_FIRE ||
        id == BLOCK_GLOWSTONE ||
        id == BLOCK_PUMPKIN_LIT)
    {
        return 15;
    } else if (id == BLOCK_TORCH) {
        return 14;
    } else if (id == BLOCK_FURNACE_LIT) {
        return 13;
    } else if (id == BLOCK_NETHER_PORTAL) {
        return 11;
    } else if (id == BLOCK_ORE_REDSTONE_GLOWING) {
        return 9;
    } else if (id == BLOCK_REDSTONE_TORCH_ON) {
        return 7;
    }
    // TODO: Apparently brown mushrooms glow, but I don't know if that's the case for Beta 1.7.3
    // Test this!
    return 0;
}