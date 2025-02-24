#pragma once
#include <sstream>

#include "player.h"
#include "helper.h"
#include "responses.h"
#include "world.h"
#include "coms.h"
#include "gamerules.h"
#include "labels.h"
#include "sysinfo.h"

class Command {
    public:
        static void Parse(std::string &rawCommand, Player* player);
    private:
        // Operator
        static void Chunk(Player* player);
        static void Creative(Player* player);
        static void Free();
        static void Gamerule(Player* player);
        static void Kick(Player* player);
        static void Save();
        static void Stop();
        static void Summon(Player* player);
        static void Teleport(Player* player);
        static void Time();
        
        static void Op(Player* player);
        static void Deop(Player* player);

        // Creative Player
        static void Give(Player* player);
        static void Health(Player* player);
        static void Help(Player* player);
        static void Kill(Player* player);
        static void Pose(Player* player);
        static void Sound(Player* player);
        static void Spawn(Player* player);
};