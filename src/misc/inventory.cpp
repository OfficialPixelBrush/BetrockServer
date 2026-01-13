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
            for (int i = 0; i < 2; i++) {
                rows.push_back(InventoryRow(2));
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
    //std::cout << "RowSetSlot: " << column << ", " << item << std::endl;
    if (column >= int32_t(slots.size()))
        return false;
    slots[column] = item;
    return true;
}

/**
 * @brief Attempts to add an item to a specific slot
 * 
 * @param column Slot column
 * @param item The to-be-added item. Remainder is put into amount
 * @return Return true if any items were added to the slot
 */
bool InventoryRow::AddItem(int32_t column, Item& item) {
    //std::cout << "RowSpreadItem: " << column << ", " <<  item << std::endl;
    // Invalid slot, fail
    if (column >= int32_t(slots.size()))
        return false;
    // No items to spread, win
    if (item.amount <= 0)
        return true;
    // Slot is empty, we can just put the item in, win
    if (slots[column].id == SLOT_EMPTY) {
        int32_t toAdd = std::min(MAX_STACK, int32_t(item.amount));
        slots[column].id     = item.id;
        slots[column].damage = item.damage;
        slots[column].amount = toAdd;
        item.amount -= toAdd;
        return true;
    }
    // If the items aren't the same, fail
    if (slots[column].id != item.id || slots[column].damage != item.damage)
        return false;
    // If the slot is already full, fail
    if (slots[column].amount >= MAX_STACK)
        return false;
    // Add together, win
    int32_t canAdd = MAX_STACK - slots[column].amount;
    int32_t toAdd = std::min(canAdd, int32_t(item.amount));
    slots[column].id = item.id;
    slots[column].damage = item.damage;
    slots[column].amount += toAdd;
    item.amount -= toAdd;
    return true;
}

/**
 * @brief Attempts to add an item to any free slot and spreads it to the row
 * 
 * @param item The to-be-added item. Remainder is put into amount
 * @return If addition succeeded
 */
bool InventoryRow::SpreadItem(Item& item) {
    //std::cout << "RowSpreadItem: " <<  item << std::endl;
    // Pass 1: Fill existing stacks
    for (int32_t i = 0; i < int32_t(slots.size()); i++) {
        if (slots[i].id == item.id && slots[i].damage == item.damage && 
            slots[i].amount < MAX_STACK) {
            AddItem(i, item);
            if (item.amount <= 0)
                return true;
        }
    }
    // Pass 2: Fill empty slots
    for (int32_t i = 0; i < int32_t(slots.size()); i++) {
        if (slots[i].id == SLOT_EMPTY) {
            AddItem(i, item);
            if (item.amount <= 0)
                return true;
        }
    }
    return item.amount <= 0;
}

/**
 * @brief Attempts to add an item to a specific slot
 * 
 * @param column Slot column
 * @param item The to-be-added item. Remainder is put into amount
 * @return If addition succeeded
 */
bool Inventory::AddItem(Int2 slotPos, Item& item) {
    if (slotPos.y >= int32_t(rows.size()))
        return false;
    return rows[slotPos.y].AddItem(slotPos.x, item);
}

/**
 * @brief Attempts to add an item to any free row
 * 
 * @param item The to-be-added item. Remainder is put into amount
 * @return If addition succeeded
 */
bool Inventory::SpreadItem(int32_t row, Item& item) {
    //std::cout << "InvSpreadItem: " <<  item << std::endl;
    if (row >= int32_t(rows.size()))
        return false;
    rows[row].SpreadItem(item);
    return item.amount <= 0;
}

/**
 * @brief Attempts to add an item to any free slot
 * 
 * @param item The to-be-added item. Remainder is put into amount
 * @return If addition succeeded
 */
bool Inventory::SpreadItem(Item& item) {
    for (auto& r : rows) {
        r.SpreadItem(item);
        if (item.amount <= 0)
            return true;
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
    //std::cout << "InvSetSlot: " << slotPos << ", " << item << std::endl;
    if (slotPos.y >= int32_t(rows.size()))
        return false;
    return rows[slotPos.y].SetSlot(slotPos.x, item);
}

Item* Inventory::GetSlotRef(Int2 slotPos) {
    if (slotPos.y >= int32_t(rows.size()))
        return nullptr;
    return rows[slotPos.y].GetSlotRef(slotPos.x);
}

/**
 * @brief Get the passed row by index
 * 
 * @param row Row Index
 * @return Desired row, returns 0-sized row if invalid
 */
InventoryRow Inventory::GetRow(int32_t row) {
    if (row >= int32_t(rows.size()))
        return InventoryRow(0);
    return rows[row];
}

/**
 * @brief Append the passed row
 * 
 * @param ir
 */
void Inventory::Append(InventoryRow ir) {
    rows.push_back(ir);
}

/**
 * @brief Append the passed inventory
 * 
 * @param inv 
 */
void Inventory::Append(Inventory inv) {
    rows.insert(rows.end(), inv.rows.begin(), inv.rows.end());
}


/**
 * @return If the inventory row is empty
 */
bool InventoryRow::Empty() {
    for (auto& i : slots) {
        if (i.amount > 0 || i.id > SLOT_EMPTY)
            return false;
    }
    return true;
}

/**
 * @return If the inventory is empty
 */
bool Inventory::Empty() {
    for (auto& r : rows) {
        if (!r.Empty())
            return false;
    }
    return true;
}

/**
 * @return Number of non-empty slots
 */
int32_t InventoryRow::FilledSlots() {
    int32_t total = 0;
    for(auto& i : slots) {
        if (i.amount > 0 || i.id > SLOT_EMPTY)
            total++;
    }
    return total;
}

/**
 * @return Number of non-empty slots
 */
int32_t Inventory::FilledSlots() {
    int32_t total = 0;
    for (auto& r : rows)
        total += r.FilledSlots();
    return total;
}

void InventoryRow::SetSlots(std::vector<Item> items) {
    slots = items;
}

void Inventory::SetLinearSlots(int32_t width, std::vector<Item> slots) {
    int32_t totalRows = int32_t(slots.size() + width - 1) / width; // ceil division
    rows.clear();
    for (int32_t row = 0; row < totalRows; row++) {
        InventoryRow ir(width);
        for (int32_t col = 0; col < width; col++) {
            int32_t idx = row * width + col;
            if (idx >= int32_t(slots.size())) break;
            ir.SetSlot(col, slots[idx]);
        }
        rows.push_back(ir);
    }
}