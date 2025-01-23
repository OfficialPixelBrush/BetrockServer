#pragma once
#include <iostream>
#include "helper.h"
#include "responses.h"
#include "entity.h"

#define HEALTH_MAX 20
#define STANCE_OFFSET 1.62

enum class ConnectionStatus {
    Disconnected,
    Handshake,
    LoggingIn,
    Connected
};

class Player : public Entity {
    public:
        std::string username = "";

        // Movement Stats
        double stance = 64.0f;
        bool crouching = false;

        // Spawn Stats
        Vec3 respawnPosition;
        int8_t respawnDimension;

        // Gameplay Stats
        bool creativeMode = false;
        int8_t health = HEALTH_MAX;

        // Connection Stats
        int64_t lastPacketTime = 0;
        int client_fd;
        ConnectionStatus connectionStatus = ConnectionStatus::Disconnected;

        Player(int client_fd, int &entityId, Vec3 position, int8_t dimension, Vec3 respawnPosition, int8_t respawnDimension)
            : Entity(entityId++, position, dimension),
            respawnPosition(respawnPosition),
            respawnDimension(respawnDimension), 
            client_fd(client_fd),
            connectionStatus(ConnectionStatus::Disconnected)
        {}

        void Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw = 0, float pitch = 0);
        void Respawn(std::vector<uint8_t> &response);
        void SetHealth(std::vector<uint8_t> &response, int8_t health);
        void Hurt(std::vector<uint8_t> &response, int8_t damage);
        void Kill(std::vector<uint8_t> &response);
        void PrintStats();
};