#include "player.h"

void Player::Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw, float pitch) {
    this->position = position;
    this->yaw = yaw;
    this->pitch = pitch;
    this->stance = position.y + STANCE_OFFSET;
    SendChunksAroundPlayer(response,this);
    Respond::PlayerPositionLook(response, this);
}

void Player::Respawn(std::vector<uint8_t> &response) {
    this->dimension = respawnDimension;
    Respond::Respawn(response, dimension);
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

void Player::PrintStats() {
    std::cout << username << ": " << position.x << ", " << position.y << ", " << position.z << "; " << yaw << ", " << pitch << std::endl;
}