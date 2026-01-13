#include "tileEntity.h"
#include "nbt.h"
#include <memory>

/**
 * @brief Append generic tile entity data
 * 
 * @param rootTag Nbt tag that the data will be appended onto
 */
void TileEntity::GetAsNbt(std::shared_ptr<CompoundNbtTag>& rootTag) {
    rootTag->Put(std::make_shared<IntNbtTag>("x", position.x));
    rootTag->Put(std::make_shared<IntNbtTag>("y", position.y));
    rootTag->Put(std::make_shared<IntNbtTag>("z", position.z));
    rootTag->Put(std::make_shared<StringNbtTag>("id", type));
}

/**
 * @brief Append Sign-unique data to nbt tag
 * 
 * @param rootTag Nbt tag that the data will be appended onto
 */
void SignTile::GetAsNbt(std::shared_ptr<CompoundNbtTag>& rootTag) {
    TileEntity::GetAsNbt(rootTag);
    rootTag->Put(std::make_shared<StringNbtTag>("Text1", lines[0]));
    rootTag->Put(std::make_shared<StringNbtTag>("Text2", lines[1]));
    rootTag->Put(std::make_shared<StringNbtTag>("Text3", lines[2]));
    rootTag->Put(std::make_shared<StringNbtTag>("Text4", lines[3]));
}

/**
 * @brief Append Chest-unique data to nbt tag
 * 
 * @param rootTag Nbt tag that the data will be appended onto
 */
void ChestTile::GetAsNbt(std::shared_ptr<CompoundNbtTag>& rootTag) {
    TileEntity::GetAsNbt(rootTag);
	auto nbtInventory = std::make_shared<ListNbtTag>("Items");
    rootTag->Put(nbtInventory);
    int32_t slotId = 0;
    for (auto& i : inventory.GetLinearSlots()) {
        if (i.id == SLOT_EMPTY) {
            slotId++;
            continue;
        }
        nbtInventory->Put(
            NbtItem(
                slotId++,
                i.id,
                i.amount,
                i.damage
            )
        );
    }
}