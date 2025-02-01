#include "items.h"

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

std::string GetLabel(int16_t id) {
    if (id > SLOT_EMPTY) {
        if (id < BLOCK_MAX) {
            return blockLabels[id];
        }
        if (id >= ITEM_SHOVEL_IRON) {
            return itemLabels[id-ITEM_SHOVEL_IRON];
        }
    }
    return "Invalid";
}

// Used to get blocks labels
std::array<std::string, BLOCK_MAX> blockLabels = {
    "Air",
    "Stone",
    "Grass",
    "Dirt",
    "Cobblestone",
    "Planks",
    "Sapling",
    "Bedrock",
    "Flowing Water",
    "Still Water",
    "Flowing Lava",
    "Still Lava",
    "Sand",
    "Gravel",
    "Gold Ore",
    "Iron Ore",
    "Coal Ore",
    "Log",
    "Leaves",
    "Sponge",
    "Glass",
    "Lapis Lazuli Ore",
    "Lapis Lazuli Block",
    "Dispenser",
    "Sandstone",
    "Note Block",
    "Bed",
    "Powered Rail",
    "Detector Rail",
    "Sticky Piston",
    "Cobweb",
    "Dead Shrub",
    "Dead Bush",
    "Piston",
    "Piston Head",
    "Wool", // 35
    "36",    // 36 does not exist
    "Dandelion",
    "Rose",
    "Brown Mushroom",
    "Red Mushroom",
    "Gold Block",
    "Iron Block",
    "Double Slab",
    "Slab",
    "Bricks",
    "TNT",
    "Bookshelf",
    "Moss Stone",
    "Obsidian",
    "Torch",
    "Fire",
    "Monster Spawner",
    "Wooden Stairs",
    "Chest",
    "Redstone",
    "Diamond Ore",
    "Diamond Block",
    "Crafting Table",
    "Wheat",
    "Farmland",
    "Furnace",
    "Lit Furnace",
    "Standing Sign",
    "Wooden Door",
    "Ladder",
    "Rail",
    "Cobblestone Stairs",
    "Wall-mounted Sign",
    "Lever",
    "Stone Pressure Plate",
    "Iron Door",
    "Wooden Pressure Plate",
    "Redstone Ore",
    "Lit Redstone Ore",
    "Redstone Torch",
    "Lit Redstone Torch",
    "Stone Button",
    "Snow Layer",
    "Ice",
    "Snow Block",
    "Cactus",
    "Clay",
    "Sugar Canes",
    "Jukebox",
    "Fence",
    "Pumpkin",
    "Netherrack",
    "Soul Sand",
    "Glowstone",
    "Nether Portal",
    "Jack o' Lantern",
    "Cake",
    "Repeater (off)",
    "Repeater (on)",
    "White Stained Glass", // did not exist yet
    "Trapdoor" 
};

std::array<std::string, ITEM_MAX-ITEM_MINIMUM> itemLabels = {
    "Iron Shovel",
    "Iron Pickaxe",
    "Iron Axe",
    "Flint and Steel",
    "Apple",
    "Bow",
    "Arrow",
    "Coal",
    "Diamond",
    "Iron Ingot",
    "Gold Ingot",
    "Iron Sword",
    "Wooden Sword",
    "Wooden Shovel",
    "Wooden Pickaxe",
    "Wooden Axe",
    "Stone Sword",
    "Stone Shovel",
    "Stone Pickaxe",
    "Stone Axe",
    "Diamond Sword",
    "Diamond Shovel",
    "Diamond Pickaxe",
    "Diamond Axe",
    "Stick",
    "Bowl",
    "Mushroom Stew",
    "Gold Sword",
    "Gold Shovel",
    "Gold Pickaxe",
    "Gold Axe",
    "String",
    "Feather",
    "Gunpowder",
    "Wooden Hoe",
    "Stone Hoe",
    "Iron Hoe",
    "Diamond Hoe",
    "Gold Hoe",
    "Wheat Seeds",
    "Wheat",
    "Bread",
    "Leather Helmet",
    "Leather Chestplate",
    "Leather Leggings",
    "Leather Boots",
    "Chainmail Helmet",
    "Chainmail Chestplate",
    "Chainmail Leggings",
    "Chainmail Boots",
    "Iron Helmet",
    "Iron Chestplate",
    "Iron Leggings",
    "Iron Boots",
    "Diamond Helmet",
    "Diamond Chestplate",
    "Diamond Leggings",
    "Diamond Boots",
    "Golden Helmet",
    "Golden Chestplate",
    "Golden Leggings",
    "Golden Boots",
    "Flint",
    "Raw Porkchop",
    "Cooked Porkchop",
    "Painting",
    "Golden Apple",
    "Enchanted Golden Apple",
    "Sign",
    "Wooden Door",
    "Bucket",
    "Water Bucket",
    "Lava Bucket",
    "Minecart",
    "Saddle",
    "Iron Door",
    "Redstone",
    "Snowball",
    "Boat",
    "Leather",
    "Milk Bucket",
    "Brick",
    "Clay",
    "Sugar Canes",
    "Paper",
    "Book",
    "Slimeball",
    "Minecart with Chest",
    "Minecart with Furnace",
    "Egg",
    "Compass",
    "Fishing Rod",
    "Clock",
    "Glowstone Dust",
    "Raw Fish",
    "Cooked Fish",
    "Dye",
    "Bone",
    "Sugar",
    "Cake",
    "Bed",
    "Redstone Repeater",
    "Cookie",
    "Map",
    "Shears"
};