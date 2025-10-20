#include "tileEntity.h"

void SignTile::SetText(std::array<std::string, 4> lines) {
    this->lines = lines;
}

std::array<std::string, 4> SignTile::GetText() {
    return lines;
}