#include "items.h"

bool IsHoe(uint8_t id) {
    if (
        ITEM_HOE_DIAMOMD ||
        ITEM_HOE_GOLD ||
        ITEM_HOE_IRON ||
        ITEM_HOE_STONE ||
        ITEM_HOE_WOOD
    ) {
        return true;
    }
    return false;
}

bool CanPlace(uint8_t id) {
    if (
        ITEM_SEEDS_WHEAT ||
        ITEM_BED ||
        ITEM_BOAT ||
        ITEM_BUCKET_WATER ||
        ITEM_BUCKET_LAVA ||
        ITEM_CAKE ||
        ITEM_DOOR_IRON ||
        ITEM_DOOR_WOOD
    ) {
        return true;
    }
    return false;
}