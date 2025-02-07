#include "player.h"
#include "client.h"
#include <cstdint>

void Player::Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw, float pitch) {
    this->position = position;
    this->yaw = yaw;
    this->pitch = pitch;
    this->stance = position.y + STANCE_OFFSET;
    SendChunksAroundPlayer(response,this);
    Respond::PlayerPositionLook(response, this);
}

void Player::Respawn(std::vector<uint8_t> &response) {
    this->worldId = respawnWorldId;
    Respond::Respawn(response, worldId);
    Teleport(response, respawnPosition);
    // After respawning, the health is automatically set back to the maximum health
    // The Client should do this automatically
    this->health = HEALTH_MAX;
}

void Player::SetHealth(std::vector<uint8_t> &response, int8_t health) {
    if (health > HEALTH_MAX) { health = HEALTH_MAX; }
    if (health < 0) { health = 0; }
    this->health = health;
    Respond::UpdateHealth(response, this->health);
}

void Player::Hurt(std::vector<uint8_t> &response, int8_t damage) {
    this->health = this->health - damage;
    Respond::UpdateHealth(response, this->health);
}

void Player::Kill(std::vector<uint8_t> &response) {
    SetHealth(response,0);
}

int8_t Player::SpreadToSlots(int16_t id, int8_t amount, int16_t damage) {
    // Then we check the entire inventory if we already have an item of this type
    // TODO: This doesn't work, fix it
    for (int8_t i = INVENTORY_ROW_1; i <= INVENTORY_HOTBAR_LAST; i++) {
        if (inventory[i].id == id && inventory[i].damage == damage) {
            if (inventory[i].amount + amount > 64) {
                amount -= inventory[i].amount;
                inventory[i].amount = 64;
            } else {
                inventory[i].amount += amount;
            }
        }
    }

    // First we check the hotbar
    for (int8_t i = INVENTORY_HOTBAR; i <= INVENTORY_HOTBAR_LAST; i++) {
        if (inventory[i].id == -1) {
            return i;
        }
    }
    // Then we check the inventory
    for (int8_t i = INVENTORY_ROW_1; i <= INVENTORY_ROW_LAST; i++) {
        if (inventory[i].id == -1) {
            return i;
        }
    }
    // No empty slot found!
    return -1;
}

bool Player::Give(std::vector<uint8_t> &response, int16_t item, int8_t amount, int16_t damage) {
    // Amount is not specified
    if (amount == -1) {
        if (item < BLOCK_MAX) {
            amount = 64;
        } else {
            amount = 1;
        }
    }
    // Look for empty slot
    int8_t slotId = SpreadToSlots(item,amount,damage);
    if (slotId == -1) {
        return false;
    }
    inventory[slotId] = Item { item,amount,damage };
    // TODO: This is a horrible solution, please find something better,
    // like checking if the inventory was changed, and only then sending out an UpdateInventory
    UpdateInventory(response);
    return true;
}

bool Player::UpdateInventory(std::vector<uint8_t> &response) {
    std::vector<Item> v(std::begin(inventory), std::end(inventory));
    Respond::WindowItems(response, 0, v);
    return true;
}

void Player::ChangeHeldItem(std::vector<uint8_t> &response, int16_t slotId) {
	currentHotbarSlot = (int8_t)slotId;
    Item i = GetHeldItem();
    Respond::EntityEquipment(response, entityId, EQUIPMENT_SLOT_HELD, i.id, i.damage);
}

Item Player::GetHeldItem() {
    return inventory[INVENTORY_HOTBAR + currentHotbarSlot];
}

// TODO: Implement Shift-clicking
void Player::ClickedSlot(std::vector<uint8_t> &response, int8_t windowId, int16_t slotId, bool rightClick, int16_t actionNumber, bool shift, int16_t id, int8_t amount, int16_t damage) {
    // If we've clicked outside, throw the items to the ground and clear the slot.
    if (slotId == CLICK_OUTSIDE) {
        hoveringItem = Item {-1,0,0};
        return;
    }

    // If something is being held
    if (hoveringItem.id < BLOCK_STONE) {
        Item temp = hoveringItem;
        hoveringItem = inventory[slotId];
        inventory[slotId] = temp;
    } else {
        Item temp = inventory[slotId];
        inventory[slotId] = hoveringItem;
        hoveringItem = temp;
    }
    lastClickedSlot = slotId;
}

bool Player::CanDecrementHotbar() {
    Item i = GetHeldItem();
    if (i.id > BLOCK_AIR && i.amount > 0) {
        return true;
    }
    return false;
}

void Player::DecrementHotbar() {
    Item* i = &inventory[INVENTORY_HOTBAR + currentHotbarSlot];
    i->amount--;
    if (i->amount <= 0) {
        i->id = -1;
        i->amount = 0;
        i->damage = 0;
    }
}

void Player::PrintStats() {
    std::cout << username << ": " << position.x << ", " << position.y << ", " << position.z << "; " << yaw << ", " << pitch << std::endl;
}