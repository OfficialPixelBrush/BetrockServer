#pragma once

#include "datatypes.h"
#include <vector>

class TileEntity {
    public:
        Int3 position;
        std::string type;

        TileEntity(Int3 position, std::string type)
            : position(position), type(type) {}
        virtual ~TileEntity() = default;
};

class SignTile : public TileEntity {
    public:
        std::array<std::string, 4> lines;

        SignTile(Int3 position, const std::array<std::string, 4>& lines = {})
            : TileEntity(position, "Sign"), lines(lines) {}
        void SetText(std::array<std::string, 4> lines);
        std::array<std::string, 4> GetText();
};

class ChestTile : public TileEntity {
    ChestTile(Int3 position)
        : TileEntity(position, "Chest") {}
};