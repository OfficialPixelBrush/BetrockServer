#pragma once

#include "datatypes.h"
#include "inventory.h"
#include <vector>

#define TILEENTITY_SIGN "Sign"
#define TILEENTITY_CHEST "Chest"

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
        void SetText(std::array<std::string, 4> lines);
        std::array<std::string, 4> GetText();
};

class ChestTile : public TileEntity {
    public:
        std::array<Item,INVENTORY_CHEST_SIZE> inventory;

        ChestTile(Int3 position, const std::array<Item,INVENTORY_CHEST_SIZE>& inventory = {})
            : TileEntity(position, TILEENTITY_CHEST), inventory(inventory) {}
        void SetInventory(std::array<Item,INVENTORY_CHEST_SIZE> inventory);
        std::array<Item,INVENTORY_CHEST_SIZE> GetInventory();
};