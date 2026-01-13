#include "inventory.h"

InventoryRow::InventoryRow(int32_t numberOfSlots) {
    slots.resize(numberOfSlots);
}

/**
 * @brief Construct a new Inventory object
 * 
 * @param pType The type of Inventory
 */
Inventory::Inventory(InventoryType pType) {
    type = pType;
    switch(type) {
        case INVENTORY_MAIN: // Main Inventory
            for (int i = 0; i < INVENTORY_MAIN_ROWS; i++) {
                rows.push_back(InventoryRow(INVENTORY_MAIN_COLS));
            }
            break;
        case INVENTORY_CHEST: // Chest Inventory
            for (int i = 0; i < INVENTORY_CHEST_ROWS; i++) {
                rows.push_back(InventoryRow(INVENTORY_CHEST_COLS));
            }
            break;
        case INVENTORY_MAIN_ARMOR:
            rows.push_back(InventoryRow(INVENTORY_MAIN_ARMOR_COLS));
            break;
        case INVENTORY_MAIN_CRAFTING:
            for (int i = 0; i < INVENTORY_2x2; i++) {
                rows.push_back(InventoryRow(INVENTORY_2x2));
            }
            break;
		default:
            break;
	}
}

Inventory::Inventory(Int2 pSize) {
    for (int32_t r = 0; r < pSize.y; r++) {
        rows.push_back(InventoryRow(pSize.x));
    }
}

std::vector<Item> InventoryRow::GetLinearSlots() {
    return slots;
}

std::vector<Item> Inventory::GetLinearSlots() {
    std::vector<Item> slots;
    for (auto& r : rows) {
        std::vector rs = r.GetLinearSlots();
        slots.insert(slots.end(), rs.begin(), rs.end());
    }
    return slots;
}

void InventoryRow::ClearSlots() {
    for (auto& s : slots) {
        s = Item();
    }
}

void Inventory::ClearSlots() {
    for (auto& r : rows) {
        r.ClearSlots();
    }
}

Item InventoryRow::GetSlot(int32_t column) {
    if (column >= int32_t(slots.size()))
        return Item{};
    return slots[column];
}

bool InventoryRow::SetSlot(int32_t column, Item item) {
    if (column >= int32_t(slots.size()))
        return false;
    slots[column] = item;
    return true;
}

/**
 * @brief Adds items to slot
 * 
 * @param column Slot column
 * @param item The to-be-added item. Remainder is put into amount
 * @return If addition succeeded
 */
bool InventoryRow::AddSlot(int32_t column, Item& item) {
    // Invalid slot
    if (column >= int32_t(slots.size()))
        return false;
    // Slot is empty, we can just put the item in
    if (slots[column].id == SLOT_EMPTY) {
        slots[column] = item;
        return true;
    }
    // Check if they're the same item
    if (item.id == slots[column].id && item.damage == slots[column].damage) {
        // Check how many items can be added
        if (slots[column].amount < MAX_STACK) {
            slots[column].amount += item.amount;
            // Items are added to slot
            // Remainder is put back into og item
            item.amount = slots[column].amount % MAX_STACK;
            if (slots[column].amount > MAX_STACK)
                slots[column].amount = MAX_STACK;
            return true;
        }
    }
    return false;
}

Item* InventoryRow::GetSlotRef(int32_t column) {
    if (column >= int32_t(slots.size()))
        return nullptr;
    return &slots[column];
}

Item Inventory::GetSlot(Int2 slotPos) {
    if (slotPos.y >= int32_t(rows.size()))
        return Item{};
    return rows[slotPos.y].GetSlot(slotPos.x);
}

bool Inventory::SetSlot(Int2 slotPos, Item item) {
    if (slotPos.y >= int32_t(rows.size()))
        return false;
    return rows[slotPos.y].SetSlot(slotPos.x, item);
}

Item* Inventory::GetSlotRef(Int2 slotPos) {
    if (slotPos.y >= int32_t(rows.size()))
        return nullptr;
    return rows[slotPos.y].GetSlotRef(slotPos.x);
}