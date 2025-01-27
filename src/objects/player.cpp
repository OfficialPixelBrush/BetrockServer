#include "player.h"

void Player::Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw, float pitch) {
    this->position = position;
    this->yaw = yaw;
    this->pitch = pitch;
    this->stance = position.y + STANCE_OFFSET;
    Respond::PlayerPositionLook(response, this);
    SendChunksAroundPlayer(response,this);
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

int8_t Player::FindEmptySlot(int16_t item, int16_t damage, int8_t amount) {
    // First we check the hotbar
    for (int8_t i = INVENTORY_HOTBAR; i <= INVENTORY_HOTBAR_LAST+1; i++) {
        if (inventory[i].x == -1) {
            return i;
        }
    }
    // Then we check the inventory
    for (int8_t i = INVENTORY_ROW_1; i <= INVENTORY_ROW_LAST; i++) {
        if (inventory[i].x == -1) {
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
    int8_t slotId = FindEmptySlot(item,damage,amount);
    if (slotId == -1) {
        return false;
    }
	Respond::SetSlot(response,0,slotId,item,amount,damage);
    inventory[slotId] = Int3 { item,damage,amount };
    return true;
}

void Player::PrintStats() {
    std::cout << username << ": " << position.x << ", " << position.y << ", " << position.z << "; " << yaw << ", " << pitch << std::endl;
}