#include "items.h"

bool IsHoe(uint8_t id) {
    if (
        id == ITEM_HOE_DIAMOMD ||
        id == ITEM_HOE_GOLD ||
        id == ITEM_HOE_IRON ||
        id == ITEM_HOE_STONE ||
        id == ITEM_HOE_WOOD
    ) {
        return true;
    }
    return false;
}

bool CanPlace(uint8_t id) {
    if (
        id == ITEM_SEEDS_WHEAT ||
        id == ITEM_BED ||
        id == ITEM_BOAT ||
        id == ITEM_BUCKET_WATER ||
        id == ITEM_BUCKET_LAVA ||
        id == ITEM_CAKE ||
        id == ITEM_DOOR_IRON ||
        id == ITEM_DOOR_WOOD
    ) {
        return true;
    }
    return false;
}