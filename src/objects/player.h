#pragma once
#include <cstdint>
#include <iostream>
#include "helper.h"
#include "responses.h"
#include "entity.h"
#include "coms.h"
#include "inventory.h"

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
        bool onFire = false;
        bool sitting = false;

        // Spawn Stats
        Vec3 respawnPosition;
        int8_t respawnWorldId;

        // Gameplay Stats
        bool creativeMode = false;
        int8_t health = HEALTH_MAX;
        std::vector<Int3> visibleChunks;
        std::vector<Int3> newChunks;
        //std::mutex visibleChunksMutex;
        std::mutex newChunksMutex;

        // Server-side inventory
        int16_t lastClickedSlot = 0;
        Item inventory[INVENTORY_MAX_SLOTS];
        Item hoveringItem = Item {-1,0,0};
        int8_t currentHotbarSlot = 0;

        // Connection Stats
        // TODO: Some of this stuff should really be moved into the client!
        int64_t lastPacketTime = 0;
        int client_fd;
        ConnectionStatus connectionStatus = ConnectionStatus::Disconnected;
        Vec3 lastChunkUpdatePosition;
        Vec3 lastEntityUpdatePosition;

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
        bool TryToPutInSlot(int16_t slot, int16_t &id, int8_t &amount, int16_t &damage);
        bool SpreadToSlots(int16_t item, int8_t amount, int16_t damage, int8_t preferredRange = 0);
        void ClickedSlot(std::vector<uint8_t> &response, int8_t windowId, int16_t slotId, bool rightClick, int16_t actionNumber, bool shift, int16_t id, int8_t amount, int16_t damage);
        bool Give(std::vector<uint8_t> &response, int16_t item, int8_t amount = -1, int16_t damage = 0);
        bool UpdateInventory(std::vector<uint8_t> &response);
        void ChangeHeldItem(std::vector<uint8_t> &response, int16_t slotId);
        int16_t GetHotbarSlot();
        Item GetHeldItem();
        bool CanDecrementHotbar();
        void DecrementHotbar(std::vector<uint8_t> &response);
        void PrintStats();
        void Save();
        void Load();
};