#pragma once
#include <iostream>
#include "helper.h"
#include "responses.h"

class Entity {
    public:
        // Movement Stats
        Vec3 position;
        Vec3 previousPosition;
        bool onGround = true;
        float yaw = 0.0f;
        float pitch = 0.0f;
        int8_t dimension;

        int8_t health;

        // Connection Stats
        int32_t entityId;

        Entity(int entityId, Vec3 position, int8_t dimension)
            : entityId(entityId++), position(position), dimension(dimension) {}

        virtual ~Entity() = default;

        virtual void Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw = 0, float pitch = 0);
        virtual void SetHealth(std::vector<uint8_t> &response, int8_t health);
        virtual void Hurt(std::vector<uint8_t> &response, int8_t damage);
        virtual void Kill(std::vector<uint8_t> &response);
        virtual void PrintStats();
};