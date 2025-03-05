#pragma once
#include <cstdint>

#include "datatypes.h"
#include "items.h"
#include "directions.h"

class World;

bool IsTranslucent(int16_t id);
uint8_t GetTranslucency(int16_t id, uint8_t &skylight);
bool IsTransparent(int16_t id);
bool IsEmissive(int16_t id);
uint8_t GetEmissiveness(int16_t id);
bool IsInstantlyBreakable(int16_t id);
bool IsInteractable(int16_t id);
bool InteractWithBlock(Block* b);
bool KeepDamageOnDrop(int8_t type);
bool NoDrop(Item item);
Item GetDrop(Item item);
Block GetPlacedBlock(World* world, Int3 pos, int8_t face, int8_t playerDirection, int16_t id, int16_t damage);
void BlockToFace(Int3& pos, int8_t& direction);
void RandomTick(Block* b, Int3 pos);

#define MAX_CROP_SIZE 7

enum Blocks {
    BLOCK_AIR           = 0,
    BLOCK_STONE         = 1,
    BLOCK_GRASS         = 2,
    BLOCK_DIRT          = 3,
    BLOCK_COBBLESTONE   = 4,
    BLOCK_PLANKS        = 5,
    BLOCK_SAPLING       = 6,
    BLOCK_BEDROCK       = 7,
    BLOCK_WATER_FLOWING = 8,
    BLOCK_WATER_STILL   = 9,
    BLOCK_LAVA_FLOWING  = 10,
    BLOCK_LAVA_STILL    = 11,
    BLOCK_SAND          = 12,
    BLOCK_GRAVEL        = 13,
    BLOCK_ORE_GOLD      = 14,
    BLOCK_ORE_IRON      = 15,
    BLOCK_ORE_COAL      = 16,
    BLOCK_LOG           = 17,
    BLOCK_LEAVES        = 18,
    BLOCK_SPONGE        = 19,
    BLOCK_GLASS         = 20,
    BLOCK_ORE_LAPIS_LAZULI = 21,
    BLOCK_DISPENSER     = 23,
    BLOCK_SANDSTONE     = 24,
    BLOCK_NOTEBLOCK     = 25,
    BLOCK_BED           = 26,
    BLOCK_RAIL_POWERED  = 27,
    BLOCK_RAIL_DETECTOR = 28,
    BLOCK_PISTON_STICKY = 29,
    BLOCK_COBWEB        = 30,
    BLOCK_TALLGRASS     = 31,
    BLOCK_DEADBUSH      = 32,
    BLOCK_PISTON        = 33,
    BLOCK_PISTON_HEAD   = 34,
    BLOCK_WOOL          = 35,
    // There is no block 36
    BLOCK_DANDELION     = 37,
    BLOCK_ROSE          = 38,
    BLOCK_MUSHROOM_BROWN= 39,
    BLOCK_MUSHROOM_RED  = 40,
    BLOCK_GOLD          = 41,
    BLOCK_IRON          = 42,
    BLOCK_DOUBLE_SLAB   = 43,
    BLOCK_SLAB          = 44,
    BLOCK_BRICKS        = 45,
    BLOCK_TNT           = 46,
    BLOCK_BOOKSHELF     = 47,
    BLOCK_COBBLESTONE_MOSSY = 48,
    BLOCK_OBSIDIAN      = 49,
    BLOCK_TORCH         = 50,
    BLOCK_FIRE          = 51,
    BLOCK_MOB_SPAWNER   = 52,
    BLOCK_STAIRS_WOOD   = 53,
    BLOCK_CHEST         = 54,
    BLOCK_REDSTONE      = 55,
    BLOCK_ORE_DIAMOND   = 56,
    BLOCK_DIAMOND       = 57,
    BLOCK_CRAFTING_TABLE= 58,
    BLOCK_CROP_WHEAT    = 59,
    BLOCK_FARMLAND      = 60,
    BLOCK_FURNACE       = 61,
    BLOCK_FURNACE_LIT   = 62,
    BLOCK_SIGN          = 63,
    BLOCK_DOOR_WOOD     = 64,
    BLOCK_LADDER        = 65,
    BLOCK_RAIL          = 66,
    BLOCK_STAIRS_COBBLESTONE = 67,
    BLOCK_SIGN_WALL     = 68,
    BLOCK_LEVER         = 69,
    BLOCK_PRESSURE_PLATE_STONE = 70,
    BLOCK_DOOR_IRON     = 71,
    BLOCK_PRESSURE_PLATE_WOOD = 72,
    BLOCK_ORE_REDSTONE_OFF  = 73,
    BLOCK_ORE_REDSTONE_ON = 74,
    BLOCK_REDSTONE_TORCH_OFF    = 75,
    BLOCK_REDSTONE_TORCH_ON     = 76,
    BLOCK_BUTTON_STONE  = 77,
    BLOCK_SNOW_LAYER    = 78,
    BLOCK_ICE           = 79,
    BLOCK_SNOW          = 80,
    BLOCK_CACTUS        = 81,
    BLOCK_CLAY          = 82,
    BLOCK_SUGARCANE     = 83,
    BLOCK_JUKEBOX       = 84,
    BLOCK_FENCE         = 85,
    BLOCK_PUMPKIN       = 86,
    BLOCK_NETHERRACK    = 87,
    BLOCK_SOULSAND      = 88,
    BLOCK_GLOWSTONE     = 89,
    BLOCK_NETHER_PORTAL  = 90,
    BLOCK_PUMPKIN_LIT   = 91,
    BLOCK_CAKE          = 92,
    BLOCK_REDSTONE_REPEATER_OFF = 93,
    BLOCK_REDSTONE_REPEATER_ON  = 94,
    BLOCK_CHEST_LOCKED  = 95,
    // 95 is stained glass, which did not exist until either
    // the April Fools 2.0 update or officially Release 1.7.2
    BLOCK_TRAPDOOR = 96,
    // 97 - 109 were added in Beta 1.8 Prerelease
    // 110 - 115 were added in Beta 1.9 Prerelease
    // 116 - 122 were added in Release 1.0
    // 123 - 124 Redstone Lamps wered added in Release 1.2.1
    // 125 - 126 were added in Release 1.3.1
    // 127 - 136 were added in Release 1.3.1
    // 137 - 145 were added in Release 1.4.2
    // 146 - 158 were added in Release 1.5
    // etc.
    BLOCK_MAX
};