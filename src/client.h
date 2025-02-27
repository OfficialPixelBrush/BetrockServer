#pragma once

#include <algorithm> // Add this at the top for std::find
#include <iostream>
#include <cstdint>
#include <netinet/in.h>
#include <unistd.h>
#include <mutex>
#include <thread>

#include "player.h"
#include "command.h"
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

        int8_t activeWindow = INVENTORY_NONE;
        int64_t lastPacketTime = 0;
        int clientFd;
        std::atomic<ConnectionStatus> connectionStatus = ConnectionStatus::Disconnected;
        
        std::vector<Int3> visibleChunks;
        std::vector<Int3> newChunks;
        std::mutex newChunksMutex;  
        Vec3 lastChunkUpdatePosition;

        // Server-side inventory
        int16_t lastClickedSlot = 0;
        Item hoveringItem = Item {-1,0,0};
        int8_t currentHotbarSlot = 0;
        
        ssize_t Setup();
        void PrintReceived(ssize_t bytes_received, Packet packetType = Packet::KeepAlive);
        void PrintRead(Packet packetType);

        // Packets
        bool HandleKeepAlive();
        bool HandleHandshake();
        bool HandleLoginRequest();
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
        void HandleLegacyPing();
        bool HandleDisconnect();

        // Helpers
        void Respawn(std::vector<uint8_t> &response);

        void SendNewChunks();
        bool UpdatePositionForOthers(bool includeLook = true);
        bool CheckPosition(Vec3 &newPosition, double &newStance); 
        bool BlockTooCloseToPosition(Int3 position);
        void HandlePacket();
        void ProcessChunk(const Int3& position, WorldManager* wm);
        void SendChunksAroundPlayer(bool forcePlayerAsCenter = false);
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
        int GetClientFd() { return this->clientFd; }

        Client(int clientFd) : clientFd(clientFd) {}
        void HandleClient();
        void DisconnectClient(std::string disconnectMessage = "");

        bool Give(std::vector<uint8_t> &response, int16_t item, int8_t amount = -1, int16_t damage = 0);
        bool UpdateInventory(std::vector<uint8_t> &response);
        int16_t GetHotbarSlot();
        Item GetHeldItem();
        bool CanDecrementHotbar();
        void DecrementHotbar(std::vector<uint8_t> &response);

        Player* GetPlayer() { return this->player.get(); };
        void Teleport(std::vector<uint8_t> &response, Vec3 position, float yaw = 0, float pitch = 0);
        void AppendResponse(std::vector<uint8_t> &addition);
        void SendResponse(bool autoclear = false);

        bool ChunkIsVisible(Int3 pos);

        std::mutex &GetNewChunksMutex() noexcept { return this->newChunksMutex; }
        void AddNewChunk(Int3 pos) { newChunks.push_back(pos); }
};