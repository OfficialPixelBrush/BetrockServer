#pragma once
#include <sstream>

#include "player.h"
#include "helper.h"
#include "responses.h"
#include "world.h"
#include "coms.h"
#include "server.h"

class Command {
    public:
        static void Parse(std::string &rawCommand, Player* player);
    private:
        static void Time();
        static void Teleport(Player* player);
        static void Give();
        static void Health(Player* player);
        static void Kill(Player* player);
        static void Summon(Player* player);
        static void Kick(Player* player);
        static void Spawn(Player* player);
        static void Creative(Player* player);
        static void Chunk(Player* player);
        static void Stop();
};