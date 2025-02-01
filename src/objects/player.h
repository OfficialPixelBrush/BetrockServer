#pragma once
#include <iostream>
#include "helper.h"
#include "responses.h"
#include "entity.h"
#include "coms.h"
#include "client.h"

#define HEALTH_MAX 20
#define STANCE_OFFSET 1.62

#define INVENTORY_CRAFTING_RESULT = 0
#define INVENTORY_CRAFTING = 1

#define INVENTORY_HOTBAR 36
#define INVENTORY_HOTBAR_LAST 44

#define INVENTORY_HELMET 5
#define INVENTORY_CHESTPLATE 6
#define INVENTORY_LEGGINGS 7
#define INVENTORY_BOOTS 8

#define INVENTORY_ROW_1 9
#define INVENTORY_ROW_2 18
#define INVENTORY_ROW_3 27
#define INVENTORY_ROW_LAST 35

#define INVENTORY_MAX_SLOTS 45

#define INVENTORY_CHEST 0
#define INVENTORY_WORKBENCH 1
#define INVENTORY_FURNACE 2
#define INVENTORY_DISPENSER 3

#define CLICK_OUTSIDE -999

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
        int8_t respawnWorldId;

        // Gameplay Stats
        bool creativeMode = false;
        int8_t health = HEALTH_MAX;
        std::vector<Int3> visibleChunks;
        std::vector<Int3> newChunks;

        // Server-side inventory
        int16_t lastClickedSlot = 0;
        Item inventory[INVENTORY_MAX_SLOTS];
        Item hoveringItem = Item {-1,0,0};
        int8_t currentHotbarSlot = 0;

        // Connection Stats
        int64_t lastPacketTime = 0;
        int client_fd;
        ConnectionStatus connectionStatus = ConnectionStatus::Disconnected;
        Vec3 lastChunkUpdatePosition;

        Player(int client_fd, int &entityId, Vec3 position, int8_t worldId, Vec3 respawnPosition, int8_t respawnWorldId)
            : Entity(entityId++, position, worldId),
            respawnPosition(respawnPosition),
            respawnWorldId(respawnWorldId), 
            client_fd(client_fd),
            connectionStatus(ConnectionStatus::Disconnected)
        {
            // Fill inventory with empty slots
            for (int i = 0; i < INVENTORY_MAX_SLOTS; ++i) {
                inventory[i] = Item{-1, 0, 0};
            }
        }

        void Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw = 0, float pitch = 0);
        void Respawn(std::vector<uint8_t> &response);
        void SetHealth(std::vector<uint8_t> &response, int8_t health);
        void Hurt(std::vector<uint8_t> &response, int8_t damage);
        void Kill(std::vector<uint8_t> &response);
        int8_t FindEmptySlot(int16_t item, int8_t amount, int16_t damage);
        void ClickedSlot(std::vector<uint8_t> &response, int8_t windowId, int16_t slotId, bool rightClick, int16_t actionNumber, bool shift, int16_t id, int8_t amount, int16_t damage);
        bool Give(std::vector<uint8_t> &response, int16_t item, int8_t amount = -1, int16_t damage = 0);
        bool CanDecrementHotbar();
        void DecrementHotbar();
        void PrintStats();
};