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

bool IsInstantlyBreakable(int16_t id) {
    if (id == BLOCK_SAPLING ||
        id == BLOCK_DEADBUSH ||
        id == BLOCK_TALLGRASS ||
        id == BLOCK_DANDELION || 
        id == BLOCK_ROSE ||
        id == BLOCK_MUSHROOM_BROWN ||
        id == BLOCK_MUSHROOM_RED ||
        id == BLOCK_TNT ||
        id == BLOCK_TORCH ||
        id == BLOCK_FIRE ||
        id == BLOCK_REDSTONE_WIRE ||
        id == BLOCK_CROP_WHEAT ||
        id == BLOCK_REDSTONE_TORCH_OFF ||
        id == BLOCK_REDSTONE_TORCH_ON ||
        id == BLOCK_SUGARCANE ||
        id == BLOCK_REDSTONE_REPEATER_OFF ||
        id == BLOCK_REDSTONE_REPEATER_ON)
    {
        return true;
    }
    return false;
}

bool NoDrop(Item item) {
    if (item.id == BLOCK_BEDROCK ||
        item.id == BLOCK_WATER_FLOWING ||
        item.id == BLOCK_WATER_STILL || 
        item.id == BLOCK_LAVA_FLOWING ||
        item.id == BLOCK_LAVA_STILL ||
        item.id == BLOCK_GLASS ||
        item.id == BLOCK_COBWEB ||
        item.id == BLOCK_DEADBUSH ||
        item.id == BLOCK_TALLGRASS ||
        item.id == BLOCK_FIRE ||
        item.id == BLOCK_MOB_SPAWNER ||
        // TODO: Figure out the max level of wheat!!
        (item.id == BLOCK_CROP_WHEAT && item.damage < 7) || 
        item.id == BLOCK_ICE ||
        item.id == BLOCK_NETHER_PORTAL ||
        item.id == BLOCK_CAKE
    ) {
        return true;
    }
    return false;
}

Item GetDrop(Item item) {
    if (NoDrop(item)) {
        return Item{ -1, 0, 0 };
    }
    // By default, give back one of the same block
    if (item.id == BLOCK_STONE) {
        item.id = BLOCK_COBBLESTONE;
        item.damage = 0;
    }
    if (item.id == BLOCK_GRASS) {
        item.id = BLOCK_DIRT;
        item.damage = 0;
    }
    if (item.id == BLOCK_ORE_COAL) {
        item.id = ITEM_COAL;
        item.damage = 0;
    }
    if (item.id == BLOCK_LEAVES) {
        item.id = BLOCK_SAPLING;
        // 1/20 chance
        item.amount = 1;
    }
    if (item.id == BLOCK_ORE_LAPIS_LAZULI) {
        item.id = ITEM_DYE;
        // 4-8
        item.amount = 4;
        item.damage = 4;
    }
    if (item.id == BLOCK_BED) {
        item.id = ITEM_BED;
        item.damage = 0;
    }
    if (item.id == BLOCK_REDSTONE_WIRE) {
        item.id = ITEM_REDSTONE;
        item.damage = 0;
    }
    if (item.id == BLOCK_ORE_DIAMOND) {
        item.id = ITEM_DIAMOND;
        item.damage = 0;
    }
    if (item.id == BLOCK_CROP_WHEAT) {
        item.id = ITEM_WHEAT;
        item.damage = 0;
    }
    if (item.id == BLOCK_SIGN_STANDING || item.id == BLOCK_SIGN_WALL) {
        item.id = ITEM_SIGN;
        item.damage = 0;
    }
    if (item.id == BLOCK_DOOR_WOOD) {
        item.id = ITEM_DOOR_WOODEN;
        item.damage = 0;
    }
    if (item.id == BLOCK_DOOR_IRON) {
        item.id = ITEM_DOOR_IRON;
        item.damage = 0;
    }
    if (item.id == BLOCK_ORE_REDSTONE || item.id == BLOCK_ORE_REDSTONE_GLOWING) {
        item.id = ITEM_REDSTONE;
        // 4-5
        item.amount = 4;
        item.damage = 0;
    }
    if (item.id == BLOCK_REDSTONE_TORCH_OFF || item.id == BLOCK_REDSTONE_TORCH_ON) {
        item.id = BLOCK_REDSTONE_TORCH_ON;
        item.damage = 0;
    }
    if (item.id == BLOCK_CLAY) {
        item.id = ITEM_CLAY;
        // ???
        item.amount = 4;
        item.damage = 0;
    }
    if (item.id == BLOCK_GLOWSTONE) {
        item.id = ITEM_GLOWSTONE_DUST;
        // 2-4
        item.damage = 2;
    }
    if (item.id == BLOCK_REDSTONE_REPEATER_ON || item.id == BLOCK_REDSTONE_REPEATER_OFF) {
        item.id = BLOCK_REDSTONE_REPEATER_OFF;
        item.damage = 0;
    }
    return item;
}