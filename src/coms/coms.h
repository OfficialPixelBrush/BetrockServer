#pragma once
#include <iostream>
#include <cstdint>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <mutex>

#include "player.h"
#include "debug.h"
#include "responses.h"
#include "world.h"
#include "packets.h"

void SendToPlayer(std::vector<uint8_t> &response, Player* player);
void BroadcastToPlayers(std::vector<uint8_t> &response, Player* sender = nullptr);
void Disconnect(Player* player, std::string message = "");
void DisconnectAllPlayers(std::string message = "");