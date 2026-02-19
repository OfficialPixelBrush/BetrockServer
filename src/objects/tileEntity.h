#pragma once

#include "datatypes.h"
#include "inventory.h"
#include "nbt.h"
#include <memory>
#include <vector>
#include <array>
#include <string>

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
        virtual void GetAsNbt(std::shared_ptr<CompoundNbtTag>& tag);
};

class SignTile : public TileEntity {
    public:
        std::array<std::string, 4> lines;

        SignTile(Int3 pPosition, const std::array<std::string, 4>& pLines = {})
            : TileEntity(pPosition, TILEENTITY_SIGN), lines(pLines) {}
        void SetText(std::array<std::string, 4> pLines) { this->lines = pLines; }
        std::array<std::string, 4> GetText() { return lines; }
        void GetAsNbt(std::shared_ptr<CompoundNbtTag>& tag) override;
};

class ChestTile : public TileEntity {
    public:
        Inventory inventory = Inventory(INVENTORY_CHEST);

        ChestTile(Int3 pPosition, const Inventory& pInv = Inventory(INVENTORY_CHEST))
            : TileEntity(pPosition, TILEENTITY_CHEST), inventory(pInv) {}
        void GetAsNbt(std::shared_ptr<CompoundNbtTag>& tag) override;
};

class MobSpawnerTile : public TileEntity {
    public:
        std::string mobLabel = "";

        MobSpawnerTile(Int3 pPosition, std::string pMobLabel)
            : TileEntity(pPosition, TILEENTITY_MOBSPAWNER), mobLabel(pMobLabel) {}
        void SetMobLabel(std::string pMobLabel) { this->mobLabel = pMobLabel; } 
        std::string GetMobLabel() { return this->mobLabel; }
};

class DispenserTile : public TileEntity {
    public:
        Inventory inventory = Inventory(INVENTORY_DISPENSER);

        DispenserTile(Int3 pPosition)
            : TileEntity(pPosition, TILEENTITY_DISPENSER) {}
};

class FurnaceTile : public TileEntity {
    public:
        Inventory inventory = Inventory(INVENTORY_FURNACE);

        FurnaceTile(Int3 pPosition)
            : TileEntity(pPosition, TILEENTITY_FURNACE) {}
};