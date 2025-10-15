#include "commandManager.h"
#include "command.h"

std::vector<std::shared_ptr<Command>> CommandManager::registeredCommands;

// Register all commands
void CommandManager::Init() {
    // Anyone can run these
	Register(std::make_shared<CommandHelp>());
	Register(std::make_shared<CommandVersion>());
	Register(std::make_shared<CommandList>());
	Register(std::make_shared<CommandPose>());
	Register(std::make_shared<CommandSpawn>());
    // Needs at least creative mode to run
	Register(std::make_shared<CommandTeleport>());
	Register(std::make_shared<CommandGive>());
	Register(std::make_shared<CommandHealth>());
    // Must be operator
	Register(std::make_shared<CommandUptime>());
	Register(std::make_shared<CommandTime>());
	Register(std::make_shared<CommandOp>());
	Register(std::make_shared<CommandDeop>());
	Register(std::make_shared<CommandWhitelist>());
	Register(std::make_shared<CommandKick>());
	Register(std::make_shared<CommandCreative>());
	Register(std::make_shared<CommandSound>());
	Register(std::make_shared<CommandKill>());
	Register(std::make_shared<CommandGamerule>());
	Register(std::make_shared<CommandSave>());
	Register(std::make_shared<CommandStop>());
	Register(std::make_shared<CommandFree>());
	Register(std::make_shared<CommandLoaded>());
	Register(std::make_shared<CommandUsage>());
	Register(std::make_shared<CommandSummon>());
	Register(std::make_shared<CommandPopulated>());
    std::cout << "Registered " << registeredCommands.size() << " command(s)!" << std::endl;
}

// Register a single command
void CommandManager::Register(std::shared_ptr<Command> command) noexcept {
    registeredCommands.push_back(command);
}

// Get all registered commands
std::vector<std::shared_ptr<Command>>& CommandManager::GetRegisteredCommands() noexcept {
    return registeredCommands;
}

// Parses commands and executes them
void CommandManager::Parse(std::string &rawCommand, Client* client) noexcept {
	// Set these up for command parsing
	std::string failureReason = "Syntax";
    std::vector<std::string> command;
	std::vector<uint8_t> response;

    std::string s;
    std::stringstream ss(rawCommand);

    while (getline(ss, s, ' ')) {
        // store token string in the vector
        command.push_back(s);
    }

	try {
        // TODO: Make this efficient
        for (int i = 0; i < registeredCommands.size(); i++) {
            if (registeredCommands[i]->GetLabel() == command[0]) {
                failureReason = registeredCommands[i]->Execute(command, response, client);
                break;
            }
        }
	} catch (const std::exception &e) {
		Betrock::Logger::Instance().Info(std::string(e.what()) + std::string(" on /") + rawCommand);
	}

	if (failureReason == "Syntax") {
		Respond::ChatMessage(response, "§cInvalid Syntax \"" + rawCommand + "\"");
	} else if (!failureReason.empty()) {
		Respond::ChatMessage(response, "§c" + failureReason);
	}
	client->AppendResponse(response);
}