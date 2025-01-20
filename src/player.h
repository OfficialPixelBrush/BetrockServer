#pragma once
#include <iostream>

class Player {
    public:
        std::string username = "";
        double x = 0.0f;
        double y = 66.0f;
        double z = 0.0;
        double stance = 64.0f;
        bool onGround = true;
        float yaw = 180.0f;
        float pitch = 0.0f;
        bool crouching = false;
        int32_t entityId;
        int client_fd;

        Player(int32_t entityId, int client_fd, std::string username) {
            this->entityId = entityId;
            this->client_fd = client_fd;
            this->username = username;
        }

        void PrintStats() {
            std::cout << username << ": " << x << ", " << y << ", " << z << "; " << yaw << ", " << pitch << std::endl;
        }
};