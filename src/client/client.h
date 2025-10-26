#pragma once

#include <algorithm> // Add this at the top for std::find
#include <iostream>
#include <cstdint>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>
#include <thread>
#include <ranges>

#include "player.h"
#include "commandManager.h"
#include "gamerules.h"
#include "labels.h"
#include "worldManager.h"
#include "version.h"
#include "helper.h"

#include "inventory.h"
#include "blocks.h"
#include "packets.h"
#include "sounds.h"
#include "animations.h"

#define PACKET_MAX 4096

class WorldManager;

enum class ConnectionStatus {
    Disconnected,
    Handshake,
    LoggingIn,
    Connected
};

class Client : public std::enable_shared_from_this<Client> {
    private:
        std::unique_ptr<Player> player;
        int32_t previousOffset = 0;
        int32_t offset = 0;
        uint8_t message[PACKET_MAX] = {0};

        std::vector<uint8_t> response;
        std::vector<uint8_t> broadcastResponse;
        std::vector<uint8_t> broadcastOthersResponse;

        // Window type
        int8_t activeWindowType = INVENTORY_NONE;
        // Index 0 is the normal window
        int8_t windowIndex = 0;

        int64_t lastPacketTime = 0;
        int clientFd;
        std::atomic<ConnectionStatus> connectionStatus = ConnectionStatus::Disconnected;
        
        std::vector<Int3> visibleChunks;
        std::vector<Int3> newChunks;
        std::mutex newChunksMutex;  
        std::mutex responseMutex;  
        Vec3 lastChunkUpdatePosition;

        // Server-side inventory
        int16_t lastClickedSlot = 0;
        Item hoveringItem = Item {-1,0,0};
        int8_t currentHotbarSlot = 0;
        
        ssize_t Setup();
        void PrintReceived(ssize_t bytes_received, Packet packetType = Packet::KeepAlive);

        // Packets
        bool HandleKeepAlive();
        bool HandleHandshake();
        bool HandleLoginRequest(World* world);
        bool HandleChatMessage();
        bool HandleUseEntity();
        bool HandleRespawn();
        bool HandlePlayerGrounded();
        bool HandlePlayerPosition();
        bool HandlePlayerLook();
        bool HandlePlayerPositionLook();
        bool HandleHoldingChange();
        bool HandleAnimation();
        bool HandleEntityAction();
        bool HandlePlayerDigging(World* world);
        bool HandlePlayerBlockPlacement(World* world);
        bool HandleCloseWindow();
        bool HandleWindowClick();
        bool HandleUpdateSign();
        void HandleLegacyPing();
        bool HandleDisconnect();

        // Helpers
        void Respawn(std::vector<uint8_t> &response);

        int8_t GetPlayerOrientation();
        void SendNewChunks();
        bool UpdatePositionForOthers(bool includeLook = true);
        bool CheckPosition(Vec3 &newPosition, double &newStance); 
        bool BlockTooCloseToPosition(Int3 position);
        void HandlePacket();
        void ProcessChunk(const Int3& position, WorldManager* wm);
        void DetermineVisibleChunks(bool forcePlayerAsCenter = false);
        bool CheckIfNewChunksRequired();
        bool TryToPutInSlot(int16_t slot, int16_t &id, int8_t &amount, int16_t &damage);
        bool SpreadToSlots(int16_t item, int8_t amount, int16_t damage, int8_t preferredRange = 0);
        void ClickedSlot(std::vector<uint8_t> &response, int8_t windowId, int16_t slotId, bool rightClick, int16_t actionNumber, bool shift, int16_t id, int8_t amount, int16_t damage);
        void ChangeHeldItem(std::vector<uint8_t> &response, int16_t slotId);
        void ClearInventory();
    public:
        void SetConnectionStatus(ConnectionStatus status) { this->connectionStatus = status; }
        ConnectionStatus GetConnectionStatus() { return this->connectionStatus; }
        void SetClientFd(int clientFd) { this->clientFd = clientFd; }
        int &GetClientFd() { return this->clientFd; }

        Client(int clientFd) : clientFd(clientFd) {}
        void HandleClient();
        void DisconnectClient(std::string disconnectMessage = "", bool tellOthers = false, bool tellPlayer = true);

        bool Give(std::vector<uint8_t> &response, int16_t item, int8_t amount = -1, int16_t damage = 0);
        bool UpdateInventory(std::vector<uint8_t> &response);
        int16_t GetHotbarSlot();
        Item GetHeldItem();
        bool CanDecrementHotbar();
        void DecrementHotbar(std::vector<uint8_t> &response);

        Player* GetPlayer() { return this->player.get(); };
        void Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw = 0, float pitch = 0);
        void TeleportKeepView(std::vector<uint8_t> &response, Vec3 position);
        void AppendResponse(std::vector<uint8_t> &addition);
        void SendResponse(bool autoclear = false);

        bool ChunkIsVisible(Int3 pos);
        void OpenWindow(int8_t type);
        void CloseLatestWindow();

        std::mutex &GetNewChunksMutex() noexcept { return this->newChunksMutex; }
        void AddNewChunk(Int3 pos) { 
            std::lock_guard<std::mutex> lock(newChunksMutex);
            if (std::find(newChunks.begin(), newChunks.end(), pos) == newChunks.end()) {
                newChunks.push_back(pos);
            }
        }
};