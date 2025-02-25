#include "command.h"

#include "client.h"
#include "server.h"

std::vector<uint8_t> response;
std::vector<std::string> command;
std::string failureReason;

// Toggle the clients pose bits
void Command::Help(Client* client) {
}

// Toggle the clients pose bits
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
		BroadcastToClients(broadcastResponse);
		failureReason = "";
		return;
	}
}

// Play a sound at the clients location
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
		BroadcastToClients(broadcastResponse);
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

void Command::Teleport(Client* client) {
	std::vector<uint8_t> sourceResponse;
	// Set the time
	if (command.size() > 1) {
		// client that is to-be teleported
		std::string source = command[1];
		auto sourceClient = Betrock::Server::Instance().FindClientByUsername(source);
		if (!sourceClient) {
			failureReason = source + " does not exist! (Source)";
			return;
		}

		// Option 1: Target coordinates
		try {
			int32_t x = std::stoi(command[2].c_str());
			int32_t y = std::stoi(command[3].c_str());
			int32_t z = std::stoi(command[4].c_str());
			Int3 tpGoal = {x,y,z};
			sourceClient->Teleport(
				sourceResponse,
				Int3ToVec3(tpGoal)
			);
			sourceClient->AppendResponse(sourceResponse);
			auto sourcePlayer = sourceClient->GetPlayer();
			Respond::ChatMessage(response, "§7Teleported  " + sourcePlayer->username + " to (" + std::to_string(x) + ", "  + std::to_string(y) + ", " + std::to_string(z) + ")");
			failureReason = "";
			return;
		} catch (const std::exception &e) {
			// Fallthrough
		}

		// Option 2: Target client
		try {
			std::string destination = command[2];
			auto destinationClient = Betrock::Server::Instance().FindClientByUsername(destination);
			if (!destinationClient) {
				failureReason = destination + " does not exist! (Destination)";
				return;
			}
			auto destinationPlayer = destinationClient->GetPlayer();
			auto sourcePlayer = sourceClient->GetPlayer();
			sourceClient->Teleport(
				sourceResponse,
				destinationPlayer->position,
				destinationPlayer->yaw,
				destinationPlayer->pitch
			);
			sourceClient->AppendResponse(sourceResponse);
			Respond::ChatMessage(response, "§7Teleported " + sourcePlayer->username + " to " + destinationPlayer->username);
		} catch (const std::exception &e) {
			failureReason = e.what();
		}
		failureReason = "";
	}
}

// Give any Item
void Command::Give(Client* client) {
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
			bool result = client->Give(response,itemId,amount,metadata);
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

// Set the clients health
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

// Kill the current client
void Command::Kill(Player* player) {
	if (command.size() > 0) {
		std::string username = player->username;
		if (command.size() > 1) {
			// Search for the client by username
			username = command[1];
			player = Betrock::Server::Instance().FindClientByUsername(username)->GetPlayer();
		}
		if (player) {
			player->Kill(response);
			Respond::ChatMessage(response, "§7Killed " + player->username);
			failureReason = "";
			return;
		}
		failureReason = "client \"" + username + "\" does not exist";
	}
}

void Command::Summon(Client* client) {
	auto &server = Betrock::Server::Instance();

	if (command.size() > 1) {
		std::scoped_lock lock(server.GetEntityIdMutex());
		std::string username = command[1];
		std::vector<uint8_t> broadcastResponse;
		Respond::NamedEntitySpawn(broadcastResponse, server.GetLatestEntityId(), username, Vec3ToInt3(client->GetPlayer()->position), 0,0, 5);
		BroadcastToClients(broadcastResponse);
		Respond::ChatMessage(response, "§7Summoned " + username);
		failureReason = "";
	}
}


// Set gamerules
void Command::Gamerule(Client* client) {
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

void Command::Kick(Client* client) {
	if (command.size() > 0) {
		std::string username = client->GetPlayer()->username;
		if (command.size() > 1) {
			// Search for the client by username
			username = command[1];
			client = Betrock::Server::Instance().FindClientByUsername(username);
		}
		if (client) {
			client->HandleDisconnect("Kicked by " + client->GetPlayer()->username);
			//Respond::ChatMessage(response, "§7Kicked " + kicked->username);
			failureReason = "";
			return;
		}
		failureReason = "client \"" + username + "\" does not exist";
	}
}

void Command::Spawn(Client* client) {
	client->Teleport(response, Betrock::Server::Instance().GetSpawnPoint());
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

void Command::Op(Client* client) {
	if (command.size() > 0) {
		Respond::ChatMessage(response, "§7Opping " + client->GetPlayer()->username);
		failureReason = "";
	}
}

void Command::Deop(Client* client) {
	if (command.size() > 0) {
		Respond::ChatMessage(response, "§7De-opping " + client->GetPlayer()->username);
		failureReason = "";
	}
}

// Parses commands and executes them
void Command::Parse(std::string &rawCommand, Client* client) {
	auto player = client->GetPlayer();
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
		} else if (command[0] == "help") {
			Help(client);
		} else if (command[0] == "op") {
			Op(client);
		} else if (command[0] == "deop") {
			Deop(client);
		} else if (command[0] == "tp") {
			Teleport(client);
		} else if (command[0] == "pose") {
			Pose(player);
		} else if (command[0] == "sound") {
			Sound(player);
		} else if (command[0] == "give") {
			Give(client);
		} else if (command[0] == "health") {
			Health(player);
		} else if (command[0] == "kill") {
			Kill(player);
		} else if (command[0] == "summon") {
			Summon(client);
		} else if (command[0] == "gamerule") {
			Gamerule(client);
		} else if (command[0] == "kick") {
			Kick(client);
		} else if (command[0] == "spawn") {
			Spawn(client);
		} else if (command[0] == "creative") {
			Creative(player);
		} else if (command[0] == "save") {
			Save();
		} else if (command[0] == "stop") {
			Stop();
		} else if (command[0] == "free") {
			Free();
		} else if (command[0] == "loaded") {
			failureReason = std::to_string(Betrock::Server::Instance().GetWorldManager(player->dimension)->world.GetNumberOfChunks());
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
	client->AppendResponse(response);
}