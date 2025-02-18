#include "player.h"
#include "client.h"
#include <cstdint>

void Player::Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw, float pitch) {
    this->position = position;
    this->yaw = yaw;
    this->pitch = pitch;
    this->stance = position.y + STANCE_OFFSET;
    this->newChunks.clear();
    Respond::PlayerPositionLook(response, this);
    SendChunksAroundPlayer(response,this, true);
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

bool Player::TryToPutInSlot(int16_t slot, int16_t &id, int8_t &amount, int16_t &damage) {
    // First, try to stack into existing slots
    if (inventory[slot].id == id && inventory[slot].damage == damage) {
        // Skip the slot if its full
        if (inventory[slot].amount >= MAX_STACK) {
            return false;
        }
        // If we fill the slot, we're done
        if (inventory[slot].amount + amount <= MAX_STACK) {
            inventory[slot].amount += amount;
            return true;
        }
        // If we fill it but items remain, keep going
        amount -= (MAX_STACK - inventory[slot].amount);
        inventory[slot].amount = MAX_STACK;
        return false;
    }
    // Secondly, try to stack into empty slots
    if (inventory[slot].id == SLOT_EMPTY) {
        inventory[slot] = { id, amount, damage };
        return true;
    }
    return false;
}

bool Player::SpreadToSlots(int16_t id, int8_t amount, int16_t damage) {
    for (int8_t i = INVENTORY_HOTBAR; i <= INVENTORY_HOTBAR_LAST; i++) {
        if (TryToPutInSlot(i, id, amount, damage)) {
            return true;
        }
    }

    for (int8_t i = INVENTORY_ROW_1; i <= INVENTORY_ROW_LAST; i++) {
        if (TryToPutInSlot(i, id, amount, damage)) {
            return true;
        }
    }

    // If there are still items left, inventory is full
    return false;
}

bool Player::Give(std::vector<uint8_t> &response, int16_t item, int8_t amount, int16_t damage) {
    // Amount is not specified
    if (amount == -1) {
        if (item < BLOCK_MAX) {
            amount = MAX_STACK;
        } else {
            amount = 1;
        }
    }
    // Look for empty slot
    SpreadToSlots(item,amount,damage);
    //inventory[slotId] = Item { item,amount,damage };
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

int16_t Player::GetHotbarSlot() {
    return INVENTORY_HOTBAR + currentHotbarSlot;
}

Item Player::GetHeldItem() {
    return inventory[GetHotbarSlot()];
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

void Player::DecrementHotbar(std::vector<uint8_t> &response) {
    Item* i = &inventory[GetHotbarSlot()];
    i->amount--;
    if (i->amount <= 0) {
        i->id = -1;
        i->amount = 0;
        i->damage = 0;
    }
	Respond::SetSlot(response, 0, GetHotbarSlot(), i->id, i->amount, i->damage);
}

void Player::PrintStats() {
    std::cout << username << ": " << position.x << ", " << position.y << ", " << position.z << "; " << yaw << ", " << pitch << std::endl;
}

void Player::Save() {
    // Health
    // OnGround
    // Dimension
    // Rotation
    // Pos
    // Inventory
        // Slot
        // Id
        // Count
        // Damage
    std::filesystem::path dirPath = Betrock::GlobalConfig::Instance().Get("level-name");
    dirPath += "/players";

    if (std::filesystem::create_directories(dirPath)) {
        std::cout << "Directory created: " << dirPath << '\n';
    }

    std::filesystem::path filePath = dirPath / (username + ".dat");

    std::ofstream playerFile (filePath);
    if (!playerFile.is_open()) {
        throw std::runtime_error("Failed to save player file " + username);
    }
    playerFile.close();
}

void Player::Load() {

}