#pragma once

#include <iostream>
#include <cstdint>
#include <netinet/in.h>
#include <unistd.h>

#include "player.h"
#include "server.h"
#include "coms.h"
#include "items.h"
#include "version.h"
#include "command.h"

void HandleClient(Player* player);