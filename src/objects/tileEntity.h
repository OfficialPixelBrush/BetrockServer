#pragma once

#include "datatypes.h"
#include "inventory.h"
#include <vector>
#include <array>

#define TILEENTITY_SIGN "Sign"
#define TILEENTITY_CHEST "Chest"
#define TILEENTITY_MOBSPAWNER "MobSpawner"
#define TILEENTITY_DISPENSER "Trap"
#define TILEENTITY_FURNACE "Furnace"

class TileEntity {
    public:
        Int3 position;
        std::string type;

        TileEntity(Int3 pPosition, std::string pType)
        {
            this->position = pPosition;
            this->type = std::move(pType);
        }
        virtual ~TileEntity() = default;
};

class SignTile : public TileEntity {
    public:
        std::array<std::string, 4> lines;

        SignTile(Int3 pPosition, const std::array<std::string, 4>& pLines = {})
            : TileEntity(pPosition, TILEENTITY_SIGN), lines(pLines) {}
        void SetText(std::array<std::string, 4> pLines) { this->lines = pLines; }
        std::array<std::string, 4> GetText() { return lines; }
};

class ChestTile : public TileEntity {
    public:
        std::array<Item,INVENTORY_CHEST_SIZE> inventory;

        ChestTile(Int3 pPosition, const std::array<Item,INVENTORY_CHEST_SIZE>& pInv = {})
            : TileEntity(pPosition, TILEENTITY_CHEST), inventory(pInv) {}
        void SetInventory(std::array<Item,INVENTORY_CHEST_SIZE> pInv) { this->inventory = pInv; }
        std::array<Item,INVENTORY_CHEST_SIZE> GetInventory() { return inventory; }
        void SetSlot(int8_t pSlot, Item pItem) {
            inventory[size_t(pSlot)] = pItem;
        }
        Item GetSlot(int8_t pSlot) { 
            return inventory[size_t(pSlot)];
        }
};

class MobSpawnerTile : public TileEntity {
    public:
        std::string mobLabel = "";

        MobSpawnerTile(Int3 pPosition, std::string pMobLabel)
            : TileEntity(pPosition, TILEENTITY_MOBSPAWNER), mobLabel(pMobLabel) {}
        void SetMobLabel(std::string pMobLabel) { this->mobLabel = pMobLabel; } 
        std::string GetMobLabel() { return this->mobLabel; }
};