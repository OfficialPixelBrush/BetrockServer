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

#define ERROR_OPERATOR "Only operators can use this command!"
#define ERROR_WHITELIST "Only whitelisted players can use this command!"

class Client;

class Command {
    public:
        static void Parse(std::string &rawCommand, Client* client);
    private:
        // Operator
        static void Creative(Player* player);
        static void Free(Player* player);
        static void Gamerule(Client* client);
        static void Kick(Client* client);
        static void Save(Player* player);
        static void Stop(Player* player);
        static void Summon(Client* client);
        static void Teleport(Client* client);
        static void Time(Player* player);
        static void Uptime(Player* player);
        
        static void Op(Player* player);
        static void Deop(Player* player);
        static void Whitelist(Player* player);

        // Creative Player
        static void List();
        static void Give(Client* client);
        static void Health(Player* player);
        static void Help();
        static void Kill(Player* player);
        static void Pose(Player* player);
        static void Sound(Player* player);
        static void Spawn(Client* client);
        static void Version();
};