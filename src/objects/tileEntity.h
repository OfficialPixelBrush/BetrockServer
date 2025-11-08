#pragma once

#include "datatypes.h"
#include "inventory.h"
#include <vector>

#define TILEENTITY_SIGN "Sign"
#define TILEENTITY_CHEST "Chest"
#define TILEENTITY_MOBSPAWNER "MobSpawner"
#define TILEENTITY_DISPENSER "Trap"
#define TILEENTITY_FURNACE "Furnace"

class TileEntity {
    public:
        Int3 position;
        std::string type;

        TileEntity(Int3 position, std::string type)
        {
            this->position = position;
            this->type = std::move(type);
        }
        virtual ~TileEntity() = default;
};

class SignTile : public TileEntity {
    public:
        std::array<std::string, 4> lines;

        SignTile(Int3 position, const std::array<std::string, 4>& lines = {})
            : TileEntity(position, TILEENTITY_SIGN), lines(lines) {}
        void SetText(std::array<std::string, 4> lines) { this->lines = lines; }
        std::array<std::string, 4> GetText() { return lines; }
};

class ChestTile : public TileEntity {
    public:
        std::array<Item,INVENTORY_CHEST_SIZE> inventory;

        ChestTile(Int3 position, const std::array<Item,INVENTORY_CHEST_SIZE>& inventory = {})
            : TileEntity(position, TILEENTITY_CHEST), inventory(inventory) {}
        void SetInventory(std::array<Item,INVENTORY_CHEST_SIZE> inventory) { this->inventory = inventory; }
        std::array<Item,INVENTORY_CHEST_SIZE> GetInventory() { return inventory; }
        void SetSlot(int8_t slot, Item item) {
            inventory[slot] = item;
        }
        Item GetSlot(int8_t slot) { 
            return inventory[slot];
        }
};

class MobSpawnerTile : public TileEntity {
    public:
        std::string mobLabel = "";

        MobSpawnerTile(Int3 position, std::string mobLabel)
            : TileEntity(position, TILEENTITY_MOBSPAWNER), mobLabel(mobLabel) {}
        void SetMobLabel(std::string mobLabel) { this->mobLabel = mobLabel; } 
        std::string GetMobLabel() { return this->mobLabel; }
};