#include "command.h"

// Get and Set the time
void Command::Time(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason) {
	// Set the time
	if (command.size() > 1) {
		serverTime = std::stol(command[1].c_str());
		Respond::Time(response,serverTime);
		Respond::ChatMessage(response, "§7Set time to " + std::to_string(serverTime));
		failureReason = "";
	}
	// Get the time
	if (command.size() == 1) {
		Respond::ChatMessage(response, "§7Current Time is " + std::to_string(serverTime));
		failureReason = "";
	}
}

// Give any Item
void Command::Give(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason) {
	if (command.size() > 1) {
		int itemId = std::stoi(command[1].c_str());
		if (
			(itemId > BLOCK_AIR && itemId < BLOCK_MAX) ||
			(itemId >= ITEM_SHOVEL_IRON && itemId < ITEM_MAX)
		) {
			// TODO: Find first empty slot in inventory!!
			Respond::SetSlot(response,0,36,itemId,64,0);
			Respond::ChatMessage(response, "§7Gave " + std::to_string(itemId));
			failureReason = "";
		} else {
			failureReason = std::to_string(itemId) + " is not a valid Item Id!";
		}
	}
}

// Set the players health
void Command::Health(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player) {
	if (command.size() > 1) {
		int health = std::stoi(command[1].c_str());
		player->SetHealth(response, health);
		Respond::ChatMessage(response, "§7Set Health to " + std::to_string(player->health));
		failureReason = "";
	}
	if (command.size() == 1) {
		Respond::ChatMessage(response, "§7Health is " + std::to_string(player->health));
		failureReason = "";
	}
}

// Kill the current player
void Command::Kill(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player) {
	if (command.size() > 0) {
		std::string username = player->username;
		if (command.size() > 1) {
			// Search for the player by username
			username = command[1];
			player = FindPlayerByUsername(username);
		}
		if (player) {
			player->Kill(response);
			Respond::ChatMessage(response, "§7Killed " + player->username);
			failureReason = "";
			return;
		}
		failureReason = "Player \"" + username + "\" does not exist";
	}
}

void Command::Summon(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player) {
	if (command.size() > 1) {
		std::lock_guard<std::mutex> lock(entityIdMutex);
		std::string username = command[1];
		std::vector<uint8_t> broadcastResponse;
		Respond::NamedEntitySpawn(broadcastResponse, latestEntityId, username, Vec3ToInt3(player->position), 0,0, 5);
		BroadcastToPlayers(broadcastResponse);
		Respond::ChatMessage(response, "§7Summoned " + username);
		failureReason = "";
	}
}

void Command::SummonPlayer(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player) {
	if (command.size() > 1) {
		int mobId = std::stoi(command[1].c_str());
		Respond::MobSpawn(response, latestEntityId, mobId,Vec3ToInt3(player->position),0,0);
		Respond::ChatMessage(response, "§7Spawned " + std::to_string(mobId));
		failureReason = "";
	}
}

void Command::Kick(std::vector<uint8_t> &response, std::vector<std::string> &command, std::string &failureReason, Player* player) {
	if (command.size() > 0) {
		std::string username = player->username;
		if (command.size() > 1) {
			// Search for the player by username
			username = command[1];
			player = FindPlayerByUsername(username);
		}
		if (player) {
			Disconnect(player, "Kicked by " + player->username);
			//Respond::ChatMessage(response, "§7Kicked " + kicked->username);
			failureReason = "";
			return;
		}
		failureReason = "Player \"" + username + "\" does not exist";
	}
}

void Command::Spawn(std::vector<uint8_t> &response, std::string &failureReason, Player* player) {
	player->Teleport(response, Int3ToVec3(spawnPoint));
	failureReason = "";
}

void Command::Creative(std::vector<uint8_t> &response, std::string &failureReason, Player* player) {
	player->creativeMode = !player->creativeMode;
	Respond::ChatMessage(response, "§7Set Creative to " + std::to_string(player->creativeMode));
	failureReason = "";
}

void Command::Chunk(std::vector<uint8_t> &response, std::string &failureReason, Player* player) {
	size_t numberOfNewChunks = SendChunksAroundPlayer(response, player);
	Respond::ChatMessage(response, "§7Generated " + std::to_string(numberOfNewChunks) + " Chunks around player");
	failureReason = "";
}

void Command::Stop(std::vector<uint8_t> &response, std::string &failureReason) {
	alive = !alive;
	Respond::ChatMessage(response, "§7Stopping Server",1);
	failureReason = "";
}

// Parses commands and executes them
void Command::Parse(std::string &rawCommand, Player* player) {
	World* world = GetDimension(player->dimension);
	std::string failureReason = "Syntax";
	std::vector<uint8_t> response;

    std::string s;
    std::stringstream ss(rawCommand);
    std::vector<std::string> command;

    while (getline(ss, s, ' ')) {
        // store token string in the vector
        command.push_back(s);
    }

    if (command[0] == "time") {
		Time(response, command, failureReason);
    } else if (command[0] == "give") {
		Give(response, command, failureReason);
	} else if (command[0] == "health") {
		Health(response, command, failureReason, player);
	} else if (command[0] == "kill") {
		Kill(response, command, failureReason, player);
	} else if (command[0] == "summon") {
		Summon(response,	command, failureReason, player);
	} else if (command[0] == "summonplayer") {
		SummonPlayer(response,	command, failureReason, player);
	} else if (command[0] == "kick") {
		Kick(response, command, failureReason, player);
	} else if (command[0] == "spawn") {
		Spawn(response, failureReason, player);
	} else if (command[0] == "creative") {
		Creative(response, failureReason, player);
	} else if (command[0] == "chunk") {
		Chunk(response, failureReason, player);
	} else if (command[0] == "stop") {
		Stop(response, failureReason);
	} else {
		failureReason = "Command does not exist";
	}

	if (failureReason == "Syntax") {
		Respond::ChatMessage(response, "§cInvalid Syntax \"" + rawCommand + "\"");
	} else if (!failureReason.empty()) {
		Respond::ChatMessage(response, "§c" + failureReason);
	}
	SendToPlayer(response,player);
}