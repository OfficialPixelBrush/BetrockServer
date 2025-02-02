#include "command.h"

std::vector<uint8_t> response;
std::vector<std::string> command;
std::string failureReason;

// Get and Set the time
void Command::Time() {
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
void Command::Teleport(Player* player) {
	// Set the time
	if (command.size() > 3) {
		int32_t x = std::stol(command[1].c_str());
		int32_t y = std::stol(command[2].c_str());
		int32_t z = std::stol(command[3].c_str());
		Int3 tpGoal = {x,y,z};
		player->Teleport(response,Int3ToVec3(tpGoal));
		Respond::ChatMessage(response, "§7Teleported " + std::to_string(x) + ", "  + std::to_string(y) + ", " + std::to_string(z));
		failureReason = "";
	}
}

// Give any Item
void Command::Give(Player* player) {
	if (command.size() > 1) {
		int8_t amount = -1;
		int8_t metadata = 0;
		if (command.size() > 2) {
			metadata = std::stoi(command[2].c_str());
		}
		if (command.size() > 3) {
			metadata = std::stoi(command[2].c_str());
			amount = std::stoi(command[3].c_str());
		}
		int16_t itemId = std::stoi(command[1].c_str());
		if (
			(itemId > BLOCK_AIR && itemId < BLOCK_MAX) ||
			(itemId >= ITEM_SHOVEL_IRON && itemId < ITEM_MAX)
		) {
			// TODO: Find first empty slot in inventory!!
			bool result = player->Give(response,itemId,amount,metadata);
			if (!result) {
				failureReason = "Unable to give " + GetLabel(itemId);
				return;
			}
			Respond::ChatMessage(response, "§7Gave " + GetLabel(itemId));
			failureReason = "";
		} else {
			failureReason = std::to_string(itemId) + " is not a valid Item Id!";
		}
	}
}

// Set the players health
void Command::Health(Player* player) {
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
void Command::Kill(Player* player) {
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

void Command::Summon(Player* player) {
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


// Set gamerules
void Command::Gamerule(Player* player) {
	if (command.size() > 1) {
		if (command[1] == "doDaylightCycle") {
			doDaylightCycle = !doDaylightCycle;
			Respond::ChatMessage(response, "§7Set doDaylightCycle to " + std::to_string(doDaylightCycle));
		} else if (command[1] == "doTileDrops") {
			doTileDrops = !doTileDrops;
			Respond::ChatMessage(response, "§7Set doTileDrops to " + std::to_string(doTileDrops));
		} else {
			failureReason = "Gamerule does not exist!";
			return;
		}
		failureReason = "";
	}
}

void Command::Kick(Player* player) {
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

void Command::Spawn(Player* player) {
	player->Teleport(response, spawnPoint);
	failureReason = "";
}

void Command::Creative(Player* player) {
	player->creativeMode = !player->creativeMode;
	Respond::ChatMessage(response, "§7Set Creative to " + std::to_string(player->creativeMode));
	failureReason = "";
}

void Command::Chunk(Player* player) {
	SendChunksAroundPlayer(response, player);
	//Respond::ChatMessage(response, "§7Generated " + std::to_string(numberOfNewChunks) + " Chunks around player");
	failureReason = "";
}

void Command::Stop() {
	alive = false;
	Respond::ChatMessage(response, "§7Stopping Server",1);
	failureReason = "";
}

// Parses commands and executes them
void Command::Parse(std::string &rawCommand, Player* player) {
	World* world = GetWorld(player->worldId);

	// Set these up for command parsing
	failureReason = "Syntax";
    command.clear();
	response.clear();

    std::string s;
    std::stringstream ss(rawCommand);

    while (getline(ss, s, ' ')) {
        // store token string in the vector
        command.push_back(s);
    }

    if (command[0] == "time") {
		Time();
    } else if (command[0] == "tp") {
		Teleport(player);
    } else if (command[0] == "give") {
		Give(player);
	} else if (command[0] == "health") {
		Health(player);
	} else if (command[0] == "kill") {
		Kill(player);
	} else if (command[0] == "summon") {
		Summon(player);
	} else if (command[0] == "gamerule") {
		Gamerule(player);
	} else if (command[0] == "kick") {
		Kick(player);
	} else if (command[0] == "spawn") {
		Spawn(player);
	} else if (command[0] == "creative") {
		Creative(player);
	} else if (command[0] == "chunk") {
		Chunk(player);
	} else if (command[0] == "stop") {
		Stop();
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