#include "command.h"
#include "commandManager.h"

#include "client.h"
#include "server.h"

std::vector<uint8_t> response;
std::vector<std::string> command;
std::string failureReason;

// Base Command
Command::Command(std::string label, std::string description, std::string syntax, bool requiresOp, bool requiresCreative) {
	this->label = label;
	this->description = description;
	this->syntax = syntax;
	this->requiresOp = requiresOp;
	this->requiresCreative = requiresCreative;
}

// Check permissions for the command
std::string Command::CheckPermissions(Client* client) {
	auto player = client->GetPlayer();
	bool isOp = Betrock::Server::Instance().IsOperator(player->username);
	if (isOp) {
		return "";
	}
	if (requiresOp && !isOp) {
		return ERROR_OPERATOR;
	}
	if (requiresCreative && !player->creativeMode) {
		return ERROR_CREATIVE;
	}
	return "";
}

// Lists commands or helps with command
std::string CommandHelp::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client)
	std::vector<std::shared_ptr<Command>> registeredCommands = CommandManager::GetRegisteredCommands();
	// Get help with specific command
	if (command.size() > 1) {
		for (size_t i = 0; i < registeredCommands.size(); i++) {
			if (registeredCommands[i]->GetLabel() == command[1]) {
				Respond::ChatMessage(
					response,
					"§7" + registeredCommands[i]->GetLabel() + ": " + registeredCommands[i]->GetDescription()
				);
				// Only print syntax if it has a value
				if (!registeredCommands[i]->GetSyntax().empty()) {
					Respond::ChatMessage(
						response,
						"§7/" + registeredCommands[i]->GetLabel() + " " + registeredCommands[i]->GetSyntax()
					);
				}
				if (registeredCommands[i]->GetRequiresOperator()) {
					Respond::ChatMessage(
						response,
						"§7(Requires operator)"
					);
				}
				return "";
			}
		}
		return "Command not found!";
	// List all commands
	} else {
		Respond::ChatMessage(response, "§7-- All commands --");
		std::string msg = "§7";
		for (size_t i = 0; i < registeredCommands.size(); i++) {
			msg += registeredCommands[i]->GetLabel();
			if (i < registeredCommands.size()-1) {
				msg += ", ";
			}
			if (msg.size() > MAX_CHAT_LINE_SIZE || i == registeredCommands.size()-1) {
				Respond::ChatMessage(response, msg);
				msg = "§7";
			}
		}
		return "";
	}
	return ERROR_REASON_SYNTAX;
}

// Shows how long the server has been alive in ticks
std::string CommandUptime::Execute([[maybe_unused]] std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client)
	Respond::ChatMessage(response, "§7Uptime is " + std::to_string(Betrock::Server::Instance().GetUpTime()) + " Ticks");
	return "";
}

// Teleports player to coordinates or another player
std::string CommandTeleport::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client)
	std::vector<uint8_t> sourceResponse;
	if (command.size() > 1) {
		// client that is to-be teleported
		std::string source = command[1];
		auto sourceClient = Betrock::Server::Instance().FindClientByUsername(source);
		if (!sourceClient) {
			return source + " does not exist! (Source)";
		}

		// Option 1: Target coordinates
		try {
			float pitch = 0.0f;
			float yaw = 0.0f;
			int32_t x = std::stoi(command[2].c_str());
			int32_t y = std::stoi(command[3].c_str());
			int32_t z = std::stoi(command[4].c_str());
			if (command.size() > 6) {
				pitch = std::stof(command[5].c_str());
				yaw = std::stof(command[6].c_str());
			}
			Int3 tpGoal = {x,y,z};
			sourceClient->Teleport(
				sourceResponse,
				Int3ToVec3(tpGoal),
				yaw, pitch
			);
			sourceClient->AppendResponse(sourceResponse);
			auto sourcePlayer = sourceClient->GetPlayer();
			Respond::ChatMessage(response, "§7Teleported  " + sourcePlayer->username + " to (" + std::to_string(x) + ", "  + std::to_string(y) + ", " + std::to_string(z) + ")");
			return "";
		} catch (const std::exception &e) {
			return ERROR_REASON_PARAMETERS;
		}

		// Option 2: Target client
		try {
			std::string destination = command[2];
			auto destinationClient = Betrock::Server::Instance().FindClientByUsername(destination);
			if (!destinationClient) {
				return destination + " does not exist! (Destination)";
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
			return e.what();
		}
	}
	return ERROR_REASON_PARAMETERS;
}

// Gets or sets the current world time
std::string CommandTime::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client)
	auto &server = Betrock::Server::Instance();
	
	// Set the time
	if (command.size() > 1) {
		server.SetServerTime(std::stol(command[1]));

		Respond::Time(response, server.GetServerTime());
		Respond::ChatMessage(response, "§7Set time to " + std::to_string(server.GetServerTime()));
		return "";
	}
	// Get the time
	if (command.size() == 1) {
		Respond::ChatMessage(response, "§7Current Time is " + std::to_string(server.GetServerTime()));
		return "";
	}
	return ERROR_REASON_SYNTAX;
}

// Shows the current Server version
std::string CommandVersion::Execute([[maybe_unused]] std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client)
	Respond::ChatMessage(response, "§7Current " + std::string(PROJECT_NAME) + " version is "  + std::string(PROJECT_VERSION_FULL_STRING));
	return "";
}

// Grant a player operator privlidges
std::string CommandOp::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client)
	
	if (command.size() > 0) {
		std::string username = client->GetPlayer()->username;
		if (command.size() > 1) {
			// Search for the client by username
			username = command[1];
		}
		Betrock::Server::Instance().AddOperator(username);
		Respond::ChatMessage(response, "§7Opping " + username);
		return "";
	}
	return ERROR_REASON_SYNTAX;
}

// Revoke a players' operator privlidges
std::string CommandDeop::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client)

	if (command.size() > 0) {
		std::string username = client->GetPlayer()->username;
		if (command.size() > 1) {
			// Search for the client by username
			username = command[1];
		}
		Betrock::Server::Instance().RemoveOperator(username);
		Respond::ChatMessage(response, "§7De-opping " + username);
		return "";
	}
	return ERROR_REASON_SYNTAX;
}

// Modify the whitelist
std::string CommandWhitelist::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client)
	// TODO:
	//- whitelist off
	//- whitelist on
	if (command.size() > 1) {
		if (command.size() > 2) {
			std::string username = command[2];
			if (command[1] == "add") {
				Betrock::Server::Instance().AddWhitelist(username);
				Respond::ChatMessage(response, "§7Whitelisted " + username);
				return "";
			}
			if (command[1] == "remove") {
				Betrock::Server::Instance().RemoveWhitelist(username);
				Respond::ChatMessage(response, "§7Unwhitelisted " + username);
				return "";
			}
		}
		if (command[1] == "reload") {
			auto& server = Betrock::Server::Instance();
			server.ReadWhitelist();
			Respond::ChatMessage(response, "§7Reloaded Whitelist");
			return "";
		}
		if (command[1] == "list") {
			auto& server = Betrock::Server::Instance();
			Respond::ChatMessage(response, "§7-- Whitelisted Players --");
			std::string msg = "§7";
			auto& whitelist = server.GetWhitelist();
			for (size_t i = 0; i < whitelist.size(); i++) {
				msg += whitelist[i];
				if (i < whitelist.size()-1) {
					msg += ", ";
				}
				if (msg.size() > 40 || i == whitelist.size()-1) {
					Respond::ChatMessage(response, msg);
					msg = "§7";
				}
			}
			return "";
		}
	}
	return ERROR_REASON_PARAMETERS;
}

// Give yourself a block or item
std::string CommandGive::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);

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
			if (!client->Give(response,itemId,amount,metadata)) {
				return "Unable to give " + IdToLabel(itemId);
			}
			Respond::ChatMessage(response, "§7Gave " + IdToLabel(itemId));
			return "";
		} else {
			return std::to_string(itemId) + " is not a valid Item Id!";
		}
	}
	return ERROR_REASON_PARAMETERS;
}

// Kick a player from the server
std::string CommandKick::Execute(std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	
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
			return "";
		}
		return "Client \"" + username + "\" does not exist!";
	}
	return ERROR_REASON_SYNTAX;
}

// Get or Set Player Health
std::string CommandHealth::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	auto player = client->GetPlayer();

	if (command.size() > 1) {
		int health = std::stoi(command[1].c_str());
		player->SetHealth(response, health);
		Respond::ChatMessage(response, "§7Set Health to " + std::to_string(player->health));
		return "";
	}
	if (command.size() == 1) {
		Respond::ChatMessage(response, "§7Health is " + std::to_string(player->health));
		return "";
	}
	return ERROR_REASON_SYNTAX;
}

// List all currently online players
std::string CommandList::Execute([[maybe_unused]] std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);

	Respond::ChatMessage(response, "§7-- All players --");
	Betrock::Server::Instance().GetConnectedClientMutex();
	auto clients = Betrock::Server::Instance().GetConnectedClients();
	std::string msg = "§7";
	for (size_t i = 0; i < clients.size(); i++) {
		msg += clients[i]->GetPlayer()->username;
		if (i < clients.size()-1) {
			msg += ", ";
		}
		if (msg.size() > MAX_CHAT_LINE_SIZE || i == clients.size()-1) {
			Respond::ChatMessage(response, msg);
			msg = "§7";
		}
	}
	return "";
}

// Toggle creative mode
std::string CommandCreative::Execute([[maybe_unused]] std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	auto player = client->GetPlayer();
	player->creativeMode = !player->creativeMode;
	Respond::ChatMessage(response, "§7Set Creative to " + std::to_string(player->creativeMode));
	return "";
}

// Set the current players' pose
std::string CommandPose::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	auto player = client->GetPlayer();

	if (command.size() > 1) {
		if (command[1] == "crouch") {
			player->crouching = !player->crouching;
		} else if (command[1] == "fire") {
			player->onFire = !player->onFire;
		} else if (command[1] == "sit") {
			player->sitting = !player->sitting;
		} else {
			return "Invalid pose";
		}
		std::vector<uint8_t> broadcastResponse;
		int8_t responseByte = (player->sitting << 2 | player->crouching << 1 | player->onFire);
		Respond::ChatMessage(response, "§7Set Pose " + std::to_string((int)responseByte));
		Respond::EntityMetadata(broadcastResponse, player->entityId, responseByte);
		BroadcastToClients(broadcastResponse);
		return "";
	}
	return ERROR_REASON_PARAMETERS;
}

// Play a specified sound
std::string CommandSound::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	auto player = client->GetPlayer();

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
		return "";
	}
	return ERROR_REASON_PARAMETERS;
}

// Kill the specified player
std::string CommandKill::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	auto player = client->GetPlayer();

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
			return "";
		}
		return "Client \"" + username + "\" does not exist";
	}
	return ERROR_REASON_SYNTAX;
}

// Configure Gamerules
std::string CommandGamerule::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	
	if (command.size() > 1) {
		if (command[1] == "doDaylightCycle") {
			doDaylightCycle = !doDaylightCycle;
			Respond::ChatMessage(response, "§7Set doDaylightCycle to " + std::to_string(doDaylightCycle));
		} else if (command[1] == "doTileDrops") {
			doTileDrops = !doTileDrops;
			Respond::ChatMessage(response, "§7Set doTileDrops to " + std::to_string(doTileDrops));
		} else if (command[1] == "keepInventory") {
			keepInventory = !keepInventory;
			Respond::ChatMessage(response, "§7Set keepInventory to " + std::to_string(keepInventory));
		} else {
			return "Gamerule does not exist!";
		}
		return "";
	}
	return ERROR_REASON_PARAMETERS;
}

// Forces the server to save all loaded chunks
std::string CommandSave::Execute([[maybe_unused]] std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	
	Respond::ChatMessage(response, "§7Saving...");
	Betrock::Server::Instance().SaveAll();
	Respond::ChatMessage(response, "§7Saved");
	return "";
}

// Forces the server to stop
std::string CommandStop::Execute([[maybe_unused]] std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	
	Respond::ChatMessage(response, "§7Stopping server");
	Betrock::Server::Instance().PrepareForShutdown();
	return "";
}

// Forces the server to unload chunks nobody can see
std::string CommandFree::Execute([[maybe_unused]] std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	
	Respond::ChatMessage(response, "§7Freeing Chunks");
	Betrock::Server::Instance().FreeAll();
	Respond::ChatMessage(response, "§7Freed Chunks");
	return "";
}

// Shows the number of loaded chunks
std::string CommandLoaded::Execute([[maybe_unused]] std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	auto player = client->GetPlayer();
	
	return std::to_string(
		Betrock::Server::Instance().GetWorldManager(player->dimension)->world.GetNumberOfChunks()
	) + " Chunk(s) are loaded";
}

// Shows the current memory usage in megabytes
std::string CommandUsage::Execute([[maybe_unused]] std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	
	return GetUsedMemoryMBString() + "MB";
}

// Summon a player entity
std::string CommandSummon::Execute(std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);

	auto &server = Betrock::Server::Instance();

	if (command.size() > 1) {
		// TODO: Actually implement entities that *exist*
		std::scoped_lock lock(server.GetEntityIdMutex());
		std::string username = command[1];
		std::vector<uint8_t> broadcastResponse;
		Respond::NamedEntitySpawn(broadcastResponse, server.GetLatestEntityId(), username, Vec3ToEntityInt3(client->GetPlayer()->position), 0,0, 5);
		BroadcastToClients(broadcastResponse);
		Respond::ChatMessage(response, "§7Summoned " + username);
		return "";
	}
	return ERROR_REASON_PARAMETERS;
}

// Check the population status of the current chunk
std::string CommandPopulated::Execute([[maybe_unused]] std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	auto player = client->GetPlayer();

	// TODO: Add support for non-zero worlds
	World* w = Betrock::Server::Instance().GetWorld(0);
	if (!w) return "World does not exist!";
	std::shared_ptr<Chunk> c = w->GetChunk(int(player->position.x/16.0),int(player->position.z/16.0));
	if (!c) return "Chunk does not exist!";
	if (c->state != ChunkState::Populated) return "Chunk is not populated";
	if (c->state == ChunkState::Populated) return "Chunk is populated";
	return ERROR_REASON_ERROR;
}

// Teleport to Spawn
std::string CommandSpawn::Execute([[maybe_unused]] std::vector<std::string> command, std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	client->Teleport(response, Int3ToVec3(Betrock::Server::Instance().GetSpawnPoint()));
	return "";
}

// Open the desired interface
std::string CommandInterface::Execute(std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	
	// TODO: Tracks players open Window IDs
	if (command[1] == "craft") {
		client->OpenWindow(INVENTORY_CRAFTING_TABLE);
	} else if (command[1] == "chest") {
		client->OpenWindow(INVENTORY_CHEST);
	}
	return "";
}

// Test the region infrastructure
std::string CommandRegion::Execute(std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	
	// Read in region data
	if (command[1] == "load") {
		std::unique_ptr<RegionFile> rf =
			std::make_unique<RegionFile>(std::filesystem::current_path() / "r.0.0.mcr");
		auto root = rf->GetChunkNbt(0,0);
		if (root) {
			root->NbtPrintData();
		}
		return std::to_string(rf->freeSectors.size());
	}
	return ERROR_REASON_PARAMETERS;
}

// Get the world seed
std::string CommandSeed::Execute([[maybe_unused]] std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	
	return std::to_string(Betrock::Server::Instance().GetWorld(0)->seed);
}

// Get the latest entity id
std::string CommandEntity::Execute([[maybe_unused]] std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	auto& server = Betrock::Server::Instance();
	std::scoped_lock lock(server.GetEntityIdMutex());
	
	return "Last ID was " + std::to_string(server.GetLatestEntityId());
}

// Get the number of modified chunks
std::string CommandModified::Execute([[maybe_unused]] std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);	
	return std::to_string(Betrock::Server::Instance().GetWorld(0)->GetNumberOfModifiedChunks());
}

// Send a custom packet
std::string CommandPacket::Execute([[maybe_unused]] std::vector<std::string> command, [[maybe_unused]] std::vector<uint8_t>& response, Client* client) {
	DEFINE_PERMSCHECK(client);
	if (command.size() < 2) return ERROR_REASON_PARAMETERS;
	std::vector<uint8_t> broadcast;
	bool broadcastToAll = false;
	// 0th is "packet"
	size_t part = 1;
	// Send this to all
	if (command[1] == "broadcast") {
		broadcastToAll = true;
		part++;
	}
	for (part = part; part < command.size(); part++) {
		unsigned int x;   
		std::stringstream ss;
		ss << std::hex << command[part];
        if (!(ss >> x) || x > 0xFF)
            return "Invalid hex value"; // or handle error

		std::cout << int(part) << ": " << int(x) << " - " << command[part] << std::endl;
        if (broadcastToAll) broadcast.push_back(static_cast<uint8_t>(x));
        else response.push_back(static_cast<uint8_t>(x));
	}
	if (broadcastToAll) BroadcastToClients(broadcast);
	return "";
}