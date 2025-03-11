#pragma once
#include <cstdint>

enum class EntityType : uint8_t  {
    Player = 1,
    DroppedItem = 2,
    Creeper = 50,
    Skeleton = 51,
    Spider = 52,
    GiantZombie = 53,
    Zombie = 54,
    Slime = 55,
    Ghast = 56,
    ZombiePigman = 57,
    Pig = 90,
    Sheep = 91,
    Cow = 92,
    Chicken = 93,
    Squid = 94,
    Wolf = 95
};

// TODO: These need to get their ID translated
// Only the client cares about keeping these separate,
// we don't. I'd prefer to keep entities and vehicles as one.
enum class ObjectType : uint8_t  {
    Boat = 1,
    Minecart = 10,
    ChestMinecart = 11,
    FurnaceMinecart = 12,
    ActivatedTNT = 50,
    Arrow = 60,
    Snowball = 61,
    Egg = 62,
    FallingSand = 70,
    FallingGravel = 71,
    FishingRodBobber = 90
};