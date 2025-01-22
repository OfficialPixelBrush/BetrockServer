#pragma once
#include <iostream>
#include "helper.h"
#include "responses.h"

#define HEALTH_MAX 20

enum ConnectionStatus {
    Disconnected,
    Handshake,
    LoggingIn,
    Connected
};

class Player {
    public:
        std::string username = "";

        // Movement Stats
        Vec3 position;
        double stance = 64.0f;
        bool onGround = true;
        float yaw = 0.0f;
        float pitch = 0.0f;
        bool crouching = false;
        int8_t dimension;

        // Spawn Stats
        Vec3 respawnPosition;
        int8_t respawnDimension;

        // Gameplay Stats
        bool creativeMode = false;
        int8_t health = HEALTH_MAX;

        // Connection Stats
        int64_t lastPacketTime = 0;
        int32_t entityId;
        int client_fd;
        int connectionStatus = Disconnected;

        Player(int client_fd, int &entityId, Vec3 position, int8_t dimension, Vec3 respawnPosition, int8_t respawnDimension) {
            this->client_fd = client_fd;
            this->entityId = entityId++;
            this->position = position;
            this->dimension = dimension;
            this->respawnPosition = respawnPosition;
            this->respawnDimension = respawnDimension;
        }

        void Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw = 0, float pitch = 0);
        void Respawn(std::vector<uint8_t> &response);
        void SetHealth(std::vector<uint8_t> &response, int8_t health);
        void Hurt(std::vector<uint8_t> &response, int8_t damage);
        void Kill(std::vector<uint8_t> &response);
        void PrintStats();
};