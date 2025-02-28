#pragma once
#include <sstream>

#include "client.h"
#include "helper.h"
#include "responses.h"
#include "world.h"
#include "coms.h"
#include "gamerules.h"
#include "labels.h"
#include "sysinfo.h"

class Client;

class Command {
    public:
        static void Parse(std::string &rawCommand, Client* client);
    private:
        // Operator
        static void Creative(Player* player);
        static void Free();
        static void Gamerule(Client* client);
        static void Kick(Client* client);
        static void Save();
        static void Stop();
        static void Summon(Client* client);
        static void Teleport(Client* client);
        static void Time();
        
        static void Op(Client* client);
        static void Deop(Client* client);

        // Creative Player
        static void Give(Client* client);
        static void Health(Player* player);
        static void Help();
        static void Kill(Player* player);
        static void Pose(Player* player);
        static void Sound(Player* player);
        static void Spawn(Client* client);
        static void Version();
};