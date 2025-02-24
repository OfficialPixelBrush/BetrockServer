#include "command.h"

#include "client.h"
#include "server.h"

std::vector<uint8_t> response;
std::vector<std::string> command;
std::string failureReason;

// Toggle the players pose bits
void Command::Pose(Player* player) {	
	// Set the time
	if (command.size() > 1) {
		if (command[1] == "crouch") {
			player->crouching = !player->crouching;
		} else if (command[1] == "fire") {
			player->onFire = !player->onFire;
		} else if (command[1] == "sit") {
			player->sitting = !player->sitting;
		} else {
			failureReason = "Invalid pose";
			return;
		}
		std::vector<uint8_t> broadcastResponse;
		int8_t responseByte = (player->sitting << 2 | player->crouching << 1 | player->onFire);
		Respond::ChatMessage(response, "§7Set Pose " + std::to_string((int)responseByte));
		Respond::EntityMetadata(broadcastResponse, player->entityId, responseByte);
		BroadcastToPlayers(broadcastResponse);
		failureReason = "";
		return;
	}
}

// Play a sound at the players location
void Command::Sound(Player* player) {	
	// Set the time
	if (command.size() > 1) {
		int32_t sound = std::stoi(command[1]);
		int32_t extraData = 0;
		if (command.size() > 2) {
			extraData = std::stoi(command[2]);
		}
		std::vector<uint8_t> broadcastResponse;
		Respond::Soundeffect(broadcastResponse,sound,Vec3ToInt3(player->position),extraData);
		Respond::ChatMessage(response, "§7Playing Sound " + std::to_string(sound));
		BroadcastToPlayers(broadcastResponse);
		failureReason = "";
		return;
	}
}

// Get and Set the time
void Command::Time() {
	auto &server = Betrock::Server::Instance();
	auto serverTime = server.GetServerTime();
	
	// Set the time
	if (command.size() > 1) {
		server.SetServerTime(std::stol(command[1]));

		Respond::Time(response, serverTime);
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
		try {
			int32_t x = std::stoi(command[1].c_str());
			int32_t y = std::stoi(command[2].c_str());
			int32_t z = std::stoi(command[3].c_str());
			// Can only really fail on the stoi
			Int3 tpGoal = {x,y,z};
			player->Teleport(response,Int3ToVec3(tpGoal));
			Respond::ChatMessage(response, "§7Teleported " + std::to_string(x) + ", "  + std::to_string(y) + ", " + std::to_string(z));
			failureReason = "";
		} catch (const std::exception &e) {
			failureReason = "Invalid destination given!";
		}
	}
}

// Give any Item
void Command::Give(Player* player) {
	if (command.size() > 1) {
		int8_t amount = -1;
		int8_t metadata = 0;
		if (command.size() > 2) {
			metadata = SafeStringToInt(command[2].c_str());
		}
		if (command.size() > 3) {
			amount = SafeStringToInt(command[3].c_str());
		}
		int16_t itemId = SafeStringToInt(command[1].c_str());
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
			player = Betrock::Server::Instance().FindPlayerByUsername(username);
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
	auto &server = Betrock::Server::Instance();

	if (command.size() > 1) {
		std::scoped_lock lock(server.GetEntityIdMutex());
		std::string username = command[1];
		std::vector<uint8_t> broadcastResponse;
		Respond::NamedEntitySpawn(broadcastResponse, server.GetLatestEntityId(), username, Vec3ToInt3(player->position), 0,0, 5);
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
			player = Betrock::Server::Instance().FindPlayerByUsername(username);
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
	player->Teleport(response, Betrock::Server::Instance().GetSpawnPoint());
	failureReason = "";
}

void Command::Creative(Player* player) {
	player->creativeMode = !player->creativeMode;
	Respond::ChatMessage(response, "§7Set Creative to " + std::to_string(player->creativeMode));
	failureReason = "";
}

void Command::Stop() {
	Respond::ChatMessage(response, "§7Stopping server");
	Betrock::Logger::Instance().Info("Stopping server");
	Betrock::Server::Instance().Stop();
	failureReason = "";
}

void Command::Save() {
	Respond::ChatMessage(response, "§7Saving...");
	Betrock::Server::Instance().SaveAll();
	Respond::ChatMessage(response, "§7Saved");
	failureReason = "";
}

void Command::Free() {
	Respond::ChatMessage(response, "§7Freeing Chunks");
	Betrock::Server::Instance().FreeAll();
	failureReason = "";
}

// Parses commands and executes them
void Command::Parse(std::string &rawCommand, Player* player) {
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

	try {
		if (command[0] == "time") {
			Time();
		} else if (command[0] == "tp") {
			Teleport(player);
		} else if (command[0] == "pose") {
			Pose(player);
		} else if (command[0] == "sound") {
			Sound(player);
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
		} else if (command[0] == "save") {
			Save();
		} else if (command[0] == "stop") {
			Stop();
		} else if (command[0] == "free") {
			Free();
		} else if (command[0] == "loaded") {
			failureReason = std::to_string(Betrock::Server::Instance().GetWorldManager(player->worldId)->world.GetNumberOfChunks());
		} else if (command[0] == "used") {
			std::stringstream ss;
			ss << std::fixed << std::setprecision(2) << GetUsedMemoryMB();
			failureReason = ss.str() + "MB";
		} else {
			failureReason = "Command does not exist!";
		}
	} catch (const std::exception &e) {
		Betrock::Logger::Instance().Info(std::string(e.what()) + std::string(" on /") + rawCommand);
	}

	if (failureReason == "Syntax") {
		Respond::ChatMessage(response, "§cInvalid Syntax \"" + rawCommand + "\"");
	} else if (!failureReason.empty()) {
		Respond::ChatMessage(response, "§c" + failureReason);
	}
	SendToPlayer(response,player);
}