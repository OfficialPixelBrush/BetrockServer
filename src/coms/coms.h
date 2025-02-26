#pragma once
#include <iostream>
#include <cstdint>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>
#include <mutex>

#include "client.h"
#include "debug.h"
#include "responses.h"
#include "world.h"
#include "packets.h"

class Client;

void BroadcastToClients(std::vector<uint8_t> &response, Client* sender = nullptr, bool autoclear = true);
void DisconnectAllClients(std::string message = "");