#include "items.h"

std::shared_ptr<Tag> NbtItem(int8_t slot, int16_t id, int8_t count, int16_t damage) {
	auto invSlot = std::make_shared<CompoundTag>(std::to_string(static_cast<int32_t>(slot)));
	invSlot->Put(std::make_shared<ByteTag> ("Slot"  , slot));
	invSlot->Put(std::make_shared<ShortTag>("id"    , id));
	invSlot->Put(std::make_shared<ByteTag> ("Count" , count));
	invSlot->Put(std::make_shared<ShortTag>("Damage", damage));
	return invSlot;
}

bool IsHoe(int16_t id) {
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

bool CanPlace(int16_t id) {
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