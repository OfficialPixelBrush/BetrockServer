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

class Player : public Entity {
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

        void ClearInventory();

        Player(int &entityId, Vec3 position, int8_t dimension, std::string world, Vec3 spawnPosition, int8_t spawnDimension, std::string spawnWorld)
            : Entity(entityId++, position, dimension, world),
            spawnPosition(spawnPosition),
            spawnDimension(spawnDimension),
            spawnWorld(spawnWorld)
        {
            ClearInventory();
        }

        void SetHealth(std::vector<uint8_t> &response, int8_t health);
        void Hurt(std::vector<uint8_t> &response, int8_t damage);
        void Kill(std::vector<uint8_t> &response);
        void PrintStats();
        void Save();
        bool Load();
};