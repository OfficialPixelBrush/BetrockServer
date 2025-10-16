#pragma once
#include <iostream>
#include "helper.h"
#include "responses.h"

class Entity {
    public:
        // Movement Stats
        Vec3 position;
        Vec3 previousPosition;
        // Default to collision size of player
        // Centered around the current position,
        // which is at the entites' feet
        AABB collisionBox = AABB
        {
            Vec3 {
                -0.3,0.0,-0.3
            },
            Vec3 {
                0.3,1.8,0.3
            }
        };
        bool onGround = true;
        float yaw = 0.0f;
        float pitch = 0.0f;

        int8_t dimension;
        std::string world;

        int8_t health;

        // Connection Stats
        int32_t entityId;

        Entity(int entityId, Vec3 position, int8_t dimension, std::string world)
            : entityId(entityId++), position(position), dimension(dimension), world(world) {}

        virtual ~Entity() = default;

        virtual void Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw = 0, float pitch = 0);
        virtual void SetHealth(std::vector<uint8_t> &response, int8_t health);
        virtual void Hurt(std::vector<uint8_t> &response, int8_t damage);
        virtual void Kill(std::vector<uint8_t> &response);
        virtual void PrintStats();
        bool CheckCollision(Vec3 otherPos, AABB otherAABB);
        Vec3 CheckPushback(Vec3 otherPos, AABB otherAABB);
};