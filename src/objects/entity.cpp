#include "entity.h"

// Get the entitys velocity
Vec3 Entity::GetVelocity() {
    return previousPosition - position;
}

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

void Entity::SimulatePhysics(float dragHorizontal, float dragVertical, float acceleration) {
    Vec3 velocity = GetVelocity();
    velocity.x = velocity.x*(1.0-dragHorizontal);
    velocity.y = velocity.y*(1.0-dragVertical);
    velocity.z = velocity.z*(1.0-dragHorizontal);
    velocity.x = velocity.x - (acceleration*(1-(1-dragHorizontal))/dragHorizontal);
    velocity.x = velocity.y - (acceleration*(1-(1-dragVertical))/dragVertical);
    velocity.x = velocity.z - (acceleration*(1-(1-dragHorizontal))/dragHorizontal);
    position = position + velocity;
}