#pragma once
#include <cstdint>
#include <iostream>
#include "helper.h"
#include "responses.h"
#include "entity.h"
#include "coms.h"
#include "inventory.h"
#include "pixnbt.h"

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
        Item inventory[INVENTORY_MAIN_SIZE];
        // 3 = helmet
        // 2 = chestplate
        // 1 = leggings
        // 0 = boots
        Item armor[INVENTORY_ARMOR_SIZE];
        Item crafting[INVENTORY_CRAFTING_SIZE];

        Player(int32_t& pEntityId, Vec3 pPosition, int8_t pDimension, std::string pWorld, Vec3 pSpawnPosition, int8_t pSpawnDimension, std::string pSpawnWorld)
            : Entity(pEntityId, pPosition, pDimension, pWorld),
            spawnPosition(pSpawnPosition),
            spawnDimension(pSpawnDimension),
            spawnWorld(pSpawnWorld)
        {
            ++pEntityId;
        }

        Vec3 GetVelocity();
        void SetHealth(std::vector<uint8_t> &response, int8_t health);
        void Hurt(std::vector<uint8_t> &response, int8_t damage);
        void Kill(std::vector<uint8_t> &response);
        void PrintStats() override;
        void Save();
        bool Load();
};