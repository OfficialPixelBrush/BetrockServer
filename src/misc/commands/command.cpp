#include "command.h"

#include "client.h"
#include "server.h"

#include "help.h"

std::vector<uint8_t> response;
std::vector<std::string> command;
std::string failureReason;

// Send the client help pages
// Do these in markdown(?)
void Command::Help() {
	Respond::ChatMessage(response, "§7-- All commands --");
	std::string msg = "§7";
	for (int i = 0; i < commandListing.size(); i++) {
		msg += commandListing[i];
		if (i < commandListing.size()-1) {
			msg += ", ";
		}
		if (msg.size() > 40 || i == commandListing.size()-1) {
			Respond::ChatMessage(response, msg);
			msg = "§7";
		}
	}
	failureReason = "";
	return;
}

void Command::List() {
	Respond::ChatMessage(response, "§7-- All players --");
	Betrock::Server::Instance().GetConnectedClientMutex();
	auto clients = Betrock::Server::Instance().GetConnectedClients();
	std::string msg = "§7";
	for (int i = 0; i < clients.size(); i++) {
		msg += clients[i]->GetPlayer()->username;
		if (i < clients.size()-1) {
			msg += ", ";
		}
		if (msg.size() > 40 || i == clients.size()-1) {
			Respond::ChatMessage(response, msg);
			msg = "§7";
		}
	}
	failureReason = "";
	return;
}

// Send the client the current server version
void Command::Version() {
	Respond::ChatMessage(response, "§7Current " + std::string(PROJECT_NAME) + " version is "  + std::string(PROJECT_VERSION_FULL_STRING));
	failureReason = "";
	return;
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

// Check if a chunk has been populated
void Command::Populated(Player* player) {	
	// TODO: Add support for non-zero worlds
	World* w = Betrock::Server::Instance().GetWorld(0);
	if (!w) failureReason = "World does not exist!";
	Chunk* c = w->GetChunk(int(player->position.x/16.0),int(player->position.z/16.0));
	if (!c) failureReason = "Chunk does not exist!";
	if (!c->populated) failureReason = "Chunk is not populated";
	if (c->populated) failureReason = "Chunk is populated";
	return;
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

// Get the uptime
void Command::Uptime(Player* player) {
	if (!Betrock::Server::Instance().IsOperator(player->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	Respond::ChatMessage(response, "§7Uptime is " + std::to_string(Betrock::Server::Instance().GetUpTime()) + " Ticks");
	failureReason = "";
}

// Get and Set the time
void Command::Time(Player* player) {
	if (!Betrock::Server::Instance().IsOperator(player->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	auto &server = Betrock::Server::Instance();
	
	// Set the time
	if (command.size() > 1) {
		server.SetServerTime(std::stol(command[1]));

		Respond::Time(response, server.GetServerTime());
		Respond::ChatMessage(response, "§7Set time to " + std::to_string(server.GetServerTime()));
		failureReason = "";
	}
	// Get the time
	if (command.size() == 1) {
		Respond::ChatMessage(response, "§7Current Time is " + std::to_string(server.GetServerTime()));
		failureReason = "";
	}
}

// Teleport a Player
void Command::Teleport(Client* client) {
	std::vector<uint8_t> sourceResponse;
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
	if (!Betrock::Server::Instance().IsOperator(player->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
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

// Summon a player entity
void Command::Summon(Client* client) {
	if (!Betrock::Server::Instance().IsOperator(client->GetPlayer()->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	auto &server = Betrock::Server::Instance();

	if (command.size() > 1) {
		// TODO: Actually implement entities that *exist*
		std::scoped_lock lock(server.GetEntityIdMutex());
		std::string username = command[1];
		std::vector<uint8_t> broadcastResponse;
		Respond::NamedEntitySpawn(broadcastResponse, server.GetLatestEntityId(), username, Vec3ToEntityInt3(client->GetPlayer()->position), 0,0, 5);
		BroadcastToClients(broadcastResponse);
		Respond::ChatMessage(response, "§7Summoned " + username);
		failureReason = "";
	}
}

// Set gamerules
void Command::Gamerule(Client* client) {
	if (!Betrock::Server::Instance().IsOperator(client->GetPlayer()->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
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

// Kick the passed player
void Command::Kick(Client* client) {
	if (!Betrock::Server::Instance().IsOperator(client->GetPlayer()->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	if (command.size() > 0) {
		std::string username = client->GetPlayer()->username;
		if (command.size() > 1) {
			// Search for the client by username
			username = command[1];
			client = Betrock::Server::Instance().FindClientByUsername(username);
		}
		if (client) {
			client->DisconnectClient("Kicked by " + client->GetPlayer()->username);
			//Respond::ChatMessage(response, "§7Kicked " + kicked->username);
			failureReason = "";
			return;
		}
		failureReason = "client \"" + username + "\" does not exist";
	}
}

// Teleport to Spawn
void Command::Spawn(Client* client) {
	client->Teleport(response, Betrock::Server::Instance().GetSpawnPoint());
	failureReason = "";
}

// Toggle Creative mode
void Command::Creative(Player* player) {
	player->creativeMode = !player->creativeMode;
	Respond::ChatMessage(response, "§7Set Creative to " + std::to_string(player->creativeMode));
	failureReason = "";
}

// Stop the Server
void Command::Stop(Player* player) {
	if (!Betrock::Server::Instance().IsOperator(player->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	Respond::ChatMessage(response, "§7Stopping server");
	Betrock::Server::Instance().PrepareForShutdown();
	failureReason = "";
}

// Save the Server
void Command::Save(Player* player) {
	if (!Betrock::Server::Instance().IsOperator(player->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	Respond::ChatMessage(response, "§7Saving...");
	Betrock::Server::Instance().SaveAll();
	Respond::ChatMessage(response, "§7Saved");
	failureReason = "";
}

// Free unseen chunks
void Command::Free(Player* player) {
	if (!Betrock::Server::Instance().IsOperator(player->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	Respond::ChatMessage(response, "§7Freeing Chunks");
	Betrock::Server::Instance().FreeAll();
	failureReason = "";
}

// Grant a player an operator
void Command::Op(Player* player) {
	if (!Betrock::Server::Instance().IsOperator(player->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	if (command.size() > 0) {
		std::string username = player->username;
		if (command.size() > 1) {
			// Search for the client by username
			username = command[1];
		}
		Betrock::Server::Instance().AddOperator(username);
		Respond::ChatMessage(response, "§7Opping " + username);
		failureReason = "";
		return;
	}
}

// Removed operator priviliges from a player
void Command::Deop(Player* player) {
	if (!Betrock::Server::Instance().IsOperator(player->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	if (command.size() > 0) {
		std::string username = player->username;
		if (command.size() > 1) {
			// Search for the client by username
			username = command[1];
		}
		Betrock::Server::Instance().RemoveOperator(username);
		Respond::ChatMessage(response, "§7De-opping " + username);
		failureReason = "";
		return;
	}
}

// Adjust a Players Whitelist settings
void Command::Whitelist(Player* player) {
	if (!Betrock::Server::Instance().IsOperator(player->username)) {
		failureReason = ERROR_OPERATOR;
		return;
	}
	// TODO:
	/*
	- whitelist off
	- whitelist on
	- whitelist add
	- whitelist reload
	*/
	if (command.size() > 1) {
		if (command.size() > 2) {
			std::string username = command[2];
			if (command[1] == "add") {
				Betrock::Server::Instance().AddWhitelist(username);
				Respond::ChatMessage(response, "§7Whitelisted " + username);
				failureReason = "";
				return;
			}
			if (command[1] == "remove") {
				Betrock::Server::Instance().RemoveWhitelist(username);
				Respond::ChatMessage(response, "§7Unwhitelisted " + username);
				failureReason = "";
				return;
			}
		}
		if (command[1] == "reload") {
			auto& server = Betrock::Server::Instance();
			server.ReadWhitelist();
			Respond::ChatMessage(response, "§7Reloaded Whitelist");
			failureReason = "";
			return;
		}
		if (command[1] == "list") {
			auto& server = Betrock::Server::Instance();
			Respond::ChatMessage(response, "§7-- Whitelisted Players --");
			std::string msg = "§7";
			auto& whitelist = server.GetWhitelist();
			for (int i = 0; i < whitelist.size(); i++) {
				msg += whitelist[i];
				if (i < whitelist.size()-1) {
					msg += ", ";
				}
				if (msg.size() > 40 || i == whitelist.size()-1) {
					Respond::ChatMessage(response, msg);
					msg = "§7";
				}
			}
			failureReason = "";
			return;
		}
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
			Time(player);
		} else if (command[0] == "uptime") {
			Uptime(player);
		} else if (command[0] == "help") {
			Help();
		} else if (command[0] == "version") {
			Version();
		} else if (command[0] == "help") {
			Help();
		} else if (command[0] == "op") {
			Op(player);
		} else if (command[0] == "whitelist") {
			Whitelist(player);
		} else if (command[0] == "deop") {
			Deop(player);
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
		} else if (command[0] == "list") {
			List();
		} else if (command[0] == "summon") {
			Summon(client);
		} else if (command[0] == "gamerule") {
			Gamerule(client);
		} else if (command[0] == "populated") {
			Populated(player);
		} else if (command[0] == "kick") {
			Kick(client);
		} else if (command[0] == "spawn") {
			Spawn(client);
		} else if (command[0] == "creative") {
			Creative(player);
		} else if (command[0] == "save") {
			Save(player);
		} else if (command[0] == "stop") {
			Stop(player);
		} else if (command[0] == "free") {
			Free(player);
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