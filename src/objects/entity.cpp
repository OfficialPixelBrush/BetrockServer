#include "entity.h"

void Entity::Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw, float pitch) {
    this->position = position;
    this->yaw = yaw;
    this->pitch = pitch;
    //Respond::PlayerPositionLook(response, this);
}

void Entity::SetHealth(std::vector<uint8_t> &response, int8_t health) {
    this->health = health;
}

void Entity::Hurt(std::vector<uint8_t> &response, int8_t damage) {
    this->health = this->health - damage;
}

void Entity::Kill(std::vector<uint8_t> &response) {
    this->health = 0;
}

void Entity::PrintStats() {
    std::cout << entityId << ": " << GetVec3(position) << "; " << yaw << ", " << pitch << std::endl;
}