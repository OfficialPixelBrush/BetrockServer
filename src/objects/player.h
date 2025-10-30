#pragma once
#include <cstdint>
#include <iostream>
#include "helper.h"
#include "responses.h"
#include "entity.h"
#include "coms.h"
#include "inventory.h"
#include "nbt.h"

#define HEALTH_MAX 20
#define STANCE_OFFSET 1.62

enum INVENTORY_SECTION {
    INVENTORY_SECTION_MAIN = 0,
    INVENTORY_SECTION_ARMOR = 1,
    INVENTORY_SECTION_CRAFTING = 2
};

class Player : public Entity {
    private:
        int8_t InventoryMappingLocalToNbt(INVENTORY_SECTION section, int8_t slot);
        int8_t InventoryMappingNbtToLocal(INVENTORY_SECTION section, int8_t slot);
    public:
        std::string username = "";

        // Movement Stats
        double stance = 64.0f;
        bool crouching = false;
        bool onFire = false;
        bool sitting = false;

        // Spawn Stats
        Vec3 spawnPosition;
        int8_t spawnDimension;
        std::string spawnWorld;

        // Gameplay Stats
        bool creativeMode = false;
        int8_t health = HEALTH_MAX;

        // Inventory
        Item inventory[INVENTORY_MAX_SLOTS];
        // 3 = helmet
        // 2 = chestplate
        // 1 = leggings
        // 0 = boots
        Item armor[4];
        Item crafting[4];

        Player(int &entityId, Vec3 position, int8_t dimension, std::string world, Vec3 spawnPosition, int8_t spawnDimension, std::string spawnWorld)
            : Entity(entityId++, position, dimension, world),
            spawnPosition(spawnPosition),
            spawnDimension(spawnDimension),
            spawnWorld(spawnWorld)
        {}

        Vec3 GetVelocity();
        void SetHealth(std::vector<uint8_t> &response, int8_t health);
        void Hurt(std::vector<uint8_t> &response, int8_t damage);
        void Kill(std::vector<uint8_t> &response);
        void PrintStats() override;
        void Save();
        bool Load();
};