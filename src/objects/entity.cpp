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

// Check collision
bool Entity::CheckCollision(Vec3 otherPos, AABB otherAABB) {
    AABB a = CalculateAABB(this->position, this->collisionBox);
    AABB b = CalculateAABB(otherPos, otherAABB);
    // Check if bounding boxes intersect
    return (
        a.min.x <= b.max.x &&
        a.max.x >= b.min.x &&
        a.min.y <= b.max.y &&
        a.max.y >= b.min.y &&
        a.min.z <= b.max.z &&
        a.max.z >= b.min.z
    );
}

Vec3 Entity::CheckPushback(Vec3 otherPos, AABB otherAABB) {
    AABB a = CalculateAABB(this->position, this->collisionBox);
    AABB b = CalculateAABB(otherPos, otherAABB);
    float overlapX1 = a.max.x - b.min.x; // A overlaps B from left
    float overlapX2 = b.max.x - a.min.x; // A overlaps B from right

    float overlapY1 = a.max.y - b.min.y;
    float overlapY2 = b.max.y - a.min.y;

    float overlapZ1 = a.max.z - b.min.z;
    float overlapZ2 = b.max.z - a.min.z;
    
    float pushX = (overlapX1 < overlapX2) ? -overlapX1 : overlapX2;
    float pushY = (overlapY1 < overlapY2) ? -overlapY1 : overlapY2;
    float pushZ = (overlapZ1 < overlapZ2) ? -overlapZ1 : overlapZ2;

    float absX = std::abs(pushX);
    float absY = std::abs(pushY);
    float absZ = std::abs(pushZ);

    Vec3 correction { 0,0,0 };

    if (absX < absY && absX < absZ)
        correction.x = pushX;
    else if (absY < absZ)
        correction.y = pushY;
    else
        correction.z = pushZ;
    std::cout << correction << std::endl;
    return correction;
}