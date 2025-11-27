#pragma once
#include <iostream>
#include "helper.h"
#include "responses.h"

class Entity {
    public:
        // Connection Stats
        int32_t entityId;
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

        Entity(int& pEntityId, Vec3 pPosition, int8_t pDimension, std::string pWorld)
            : entityId(pEntityId), position(pPosition), dimension(pDimension), world(pWorld)
        {
            ++pEntityId;
        }

        virtual ~Entity() = default;

        virtual void Teleport(std::vector<uint8_t> &pResponse, Vec3 pPosition, float pYaw = 0, float pPitch = 0);
        virtual void SetHealth(std::vector<uint8_t> &pResponse, int8_t pHealth);
        virtual void Hurt(std::vector<uint8_t> &pResponse, int8_t pDamage);
        virtual void Kill(std::vector<uint8_t> &pResponse);
        virtual void PrintStats();
        bool CheckCollision(Vec3 pOtherPos, AABB pOtherAABB);
        Vec3 CheckPushback(Vec3 pOtherPos, AABB pOtherAABB);
};