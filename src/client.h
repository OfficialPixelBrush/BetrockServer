#pragma once

#include <algorithm> // Add this at the top for std::find
#include <iostream>
#include <cstdint>
#include <netinet/in.h>
#include <unistd.h>

#include "coms.h"
#include "player.h"
#include "command.h"
#include "worldManager.h"
#include "version.h"

#include "inventory.h"
#include "blocks.h"
#include "packets.h"
#include "sounds.h"
#include "animations.h"

#define PACKET_MAX 4096

class Client {
    public:
        Client(Player* player);
        Player* player;
        int32_t previousOffset = 0;
        int32_t offset = 0;
        char message[PACKET_MAX] = {0};

        int8_t activeWindow = INVENTORY_NONE;

        std::vector<uint8_t> response;
        std::vector<uint8_t> broadcastResponse;
        std::vector<uint8_t> broadcastOthersResponse;
        
        ssize_t Setup();
        void PrintReceived(Packet packetType, ssize_t bytes_received);
        void PrintRead(Packet packetType);

        bool KeepAlive();
        bool Handshake();
        bool LoginRequest();
        bool ChatMessage();
        bool UseEntity();
        bool Respawn();
        bool PlayerGrounded();
        bool PlayerPosition();
        bool PlayerLook();
        bool PlayerPositionLook();
        bool HoldingChange();
        bool Animation();
        bool EntityAction();
        bool PlayerDigging(World* world);
        bool PlayerBlockPlacement(World* world);
        bool CloseWindow();
        bool WindowClick();
        bool DisconnectClient();

        void Respond(ssize_t bytes_received);
        void SendNewChunks();
        bool UpdatePositionForOthers(bool includeLook = true);

    private:
        bool CheckPosition(Player* player, Vec3 &newPosition, double &newStance); 
        bool BlockTooCloseToPosition(Int3 position);
};

void HandleClient(Player* player);
void ProcessChunk(std::vector<uint8_t>& response, const Int3& position, WorldManager* wm, Player* player);
void SendChunksAroundPlayer(std::vector<uint8_t> &response, Player* player, bool forcePlayerAsCenter = false);