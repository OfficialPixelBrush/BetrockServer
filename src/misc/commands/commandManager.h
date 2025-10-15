#pragma once
#include "client.h"

class Command;
class Client;

class CommandManager {
    public:
        static void Init();
        static void Parse(std::string &rawCommand, Client* client) noexcept;
        static void Register(std::shared_ptr<Command> command) noexcept;
        static std::vector<std::shared_ptr<Command>>& GetRegisteredCommands() noexcept;
    private:
        static std::vector<std::shared_ptr<Command>> registeredCommands;
};