#include "entity.h"

// Teleport the entity to the passed position
void Entity::Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw, float pitch) {
    this->position = position;
    this->yaw = yaw;
    this->pitch = pitch;
    //Respond::PlayerPositionLook(response, this);
}

// Set the entity health
void Entity::SetHealth(std::vector<uint8_t> &response, int8_t health) {
    this->health = health;
}

// Reduce the entites health by the damage amount
void Entity::Hurt(std::vector<uint8_t> &response, int8_t damage) {
    this->health = this->health - damage;
}

// Set the entitys health to 0
void Entity::Kill(std::vector<uint8_t> &response) {
    this->health = 0;
}

// Print the entities stats
void Entity::PrintStats() {
    std::cout << entityId << ": " << position << "; " << yaw << ", " << pitch << std::endl;
}