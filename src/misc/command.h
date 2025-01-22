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
        static void Time(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason);
        static void Give(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason);
        static void Health(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player);
        static void Kill(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player);
        static void Summon(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player);
        static void SummonPlayer(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player);
        static void Kick(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player);
        static void Spawn(std::vector<uint8_t> &response, std::string &failureReason, Player* player);
        static void Creative(std::vector<uint8_t> &response, std::string &failureReason, Player* player);
        static void Chunk(std::vector<uint8_t> &response, std::string &failureReason, Player* player);
        static void Stop(std::vector<uint8_t> &response, std::string &failureReason);
};