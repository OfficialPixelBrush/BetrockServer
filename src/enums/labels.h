#pragma once
#include <cstdint>
#include <array>
#include <string>

#include "blocks.h"
#include "items.h"

extern std::array<std::string, BLOCK_MAX> blockLabels;
extern std::array<std::string, ITEM_MAX-ITEM_MINIMUM> itemLabels;

std::string GetLabel(int16_t id);