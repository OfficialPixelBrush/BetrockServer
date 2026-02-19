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
	INVENTORY_MAIN_HOTBAR = 6,
	INVENTORY_MAIN_CRAFTING = 7,
	INVENTORY_MAIN_ARMOR = 8,
	CLICK_OUTSIDE = -999
};
#define MAX_TOOL_STACK 1
#define MAX_ITEM_STACK 64
#define MAX_BLOCK_STACK 64

#define INVENTORY_3x3 (3*2)
#define INVENTORY_2x2 (2*2)

#define INVENTORY_CHEST_ROWS 3
#define INVENTORY_CHEST_LARGE_ROWS 6
#define INVENTORY_CHEST_COLS 9

#define INVENTORY_CHEST_TOTAL (INVENTORY_CHEST_ROWS * INVENTORY_CHEST_COLS)
#define INVENTORY_CHEST_LARGE_TOTAL (INVENTORY_CHEST_LARGE_ROWS * INVENTORY_CHEST_COLS)

#define INVENTORY_FURNACE_COLS 3

#define INVENTORY_MAIN_ROWS 3
#define INVENTORY_MAIN_COLS 9
#define INVENTORY_MAIN_TOTAL (INVENTORY_MAIN_ROWS * INVENTORY_MAIN_COLS)

#define INVENTORY_MAIN_ARMOR_COLS 4

#define INVENTORY_MAIN_HOTBAR_NETWORK (1 + INVENTORY_2x2 + INVENTORY_MAIN_ARMOR_COLS + INVENTORY_MAIN_TOTAL)
#define INVENTORY_MAIN_HOTBAR_COLS INVENTORY_MAIN_COLS

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
		void SetSlots(std::vector<Item> items);
		bool AddItem(int32_t column, Item& item);
		bool SpreadItem(Item& item);
		Item GetSlot(int32_t column);
		Item* GetSlotRef(int32_t column);
		std::vector<Item> GetLinearSlots();
		int32_t GetCols() { return int32_t(slots.size()); }
		void ClearSlots();
		bool Empty();
		int32_t FilledSlots();
};

class Inventory {
	private:
		InventoryType type = INVENTORY_MAIN;
		std::vector<InventoryRow> rows;
	public:
		Inventory() {}
		Inventory(InventoryType type);
		Inventory(Int2 pSize);
		bool SetSlot(Int2 slotPos, Item item);
		bool AddItem(Int2 slotPos, Item& item);
		bool SpreadItem(Item& item);
		bool SpreadItem(int32_t row, Item& item);
		Item GetSlot(Int2 slotPos);
		InventoryRow GetRow(int32_t row);
		Item* GetSlotRef(Int2 slotPos);

		int32_t GetRows() { return int32_t(rows.size()); };
		int32_t GetCols() { return rows[0].GetCols(); };
		std::vector<Item> GetLinearSlots();
		void SetLinearSlots(int32_t width, std::vector<Item> slots);
		void ClearSlots();
		void Append(InventoryRow iv);
		void Append(Inventory inv);
		bool Empty();
		int32_t FilledSlots();
		
};