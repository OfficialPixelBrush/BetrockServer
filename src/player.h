#pragma once
#include <iostream>

class Player {
    public:
        std::string username = "";
        double x = 0.0f;
        double y = 64.0f;
        double z = 0.0;
        double stance = 64.0f;
        bool onGround = true;
        float yaw = 180.0f;
        float pitch = 0.0f;

        Player(std::string username, double x, double y, double z) {
            this->username = username;
            this->x = x;
            this->y = y;
            this->z = z;
        }

        void PrintStats() {
            std::cout << username << ": " << x << ", " << y << ", " << z << "; " << yaw << ", " << pitch << std::endl;
        }
};