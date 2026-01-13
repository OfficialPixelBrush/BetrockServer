#pragma once

#include "datatypes.h"
#include <cstdint>
#include <vector>
enum InventoryType {
	INVENTORY_NONE = -1,
	INVENTORY_CHEST = 0,
	INVENTORY_CRAFTING_TABLE = 1,
	INVENTORY_FURNACE = 2,
	INVENTORY_DISPENSER = 3,
	// The following don't actually exist,
	// but makes distinction easier
	INVENTORY_CHEST_LARGE = 4,
	INVENTORY_MAIN = 5,
	INVENTORY_MAIN_CRAFTING = 6,
	INVENTORY_MAIN_ARMOR = 7,
	CLICK_OUTSIDE = -999
};
#define MAX_STACK 64

#define INVENTORY_3x3 3
#define INVENTORY_2x2 2

#define INVENTORY_CHEST_ROWS 3
#define INVENTORY_CHEST_LARGE_ROWS 6
#define INVENTORY_CHEST_COLS 9

#define INVENTORY_FURNACE_COLS 3

#define INVENTORY_MAIN_ROWS 4
#define INVENTORY_MAIN_COLS 9
#define INVENTORY_MAIN_ARMOR_COLS 4

#define INVENTORY_HOTBAR_ROW 4

// Equipment slots
#define EQUIPMENT_SLOT_HELD 0
#define EQUIPMENT_SLOT_HELMET 3
#define EQUIPMENT_SLOT_CHESTPLATE 2
#define EQUIPMENT_SLOT_LEGGINGS 1
#define EQUIPMENT_SLOT_BOOTS 0

#define SLOT_EMPTY -1

class InventoryRow {
	private:
		std::vector<Item> slots;
	public:
		InventoryRow(int32_t numberOfSlots);
		bool SetSlot(int32_t column, Item item);
		bool AddSlot(int32_t column, Item& item);
		Item GetSlot(int32_t column);
		Item* GetSlotRef(int32_t column);
		std::vector<Item> GetLinearSlots();
		int32_t GetCols() { return int32_t(slots.size()); }
		void ClearSlots();
};

class Inventory {
	private:
		InventoryType type = INVENTORY_MAIN;
		std::vector<InventoryRow> rows;
	public:
		Inventory(InventoryType type = INVENTORY_MAIN);
		Inventory(Int2 pSize = Int2{1,1});
		bool SetSlot(Int2 slotPos, Item item);
		bool AddSlot(Int2 slotPos, Item& item);
		Item GetSlot(Int2 slotPos);
		Item* GetSlotRef(Int2 slotPos);

		int32_t GetRows() { return int32_t(rows.size()); };
		int32_t GetCols() { return rows[0].GetCols(); };
		std::vector<Item> GetLinearSlots();
		void ClearSlots();
};