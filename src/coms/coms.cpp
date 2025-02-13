#include "coms.h"

#include "server.h"

// Send the contents of response to the specified Player
void SendToPlayer(std::vector<uint8_t> &response, Player* player) {
	if (response.empty() || !player || player->connectionStatus <= ConnectionStatus::Disconnected) {
		return;
	}

	if (debugSentPacketType) {
		std::cout << "Sending " << PacketIdToLabel((Packet)response[0]) << " to " << player->username << "(" << player->entityId << ")" << "! (" << response.size() << " Bytes)" << std::endl;
	}
		
	if (debugSentBytes) {
		for (uint i = 0; i < response.size(); i++) {
			std::cout << std::hex << (int)response[i];
			if (i < response.size()-1) {
				std::cout << ", ";
			}
		}
		std::cout << std::dec << std::endl;
	}
	
	ssize_t bytes_sent = send(player->client_fd, response.data(), response.size(), 0);
	if (bytes_sent == -1) {
		perror("send");
		return;
	}
}

// Sent the specified message to all currently connected Players
void BroadcastToPlayers(std::vector<uint8_t> &response, Player* sender) {
	if (response.empty()) {
		return;
	}

	auto &server = Betrock::Server::Instance();

	std::scoped_lock lock(server.GetConnectedPlayerMutex());
    for (Player *player : server.GetConnectedPlayers()) {
		if (player == sender) { continue; }
		if (player->connectionStatus == ConnectionStatus::Connected) {
        	SendToPlayer(response,player);
		}
    }
}

// Disconnects the specified client immediately
void Disconnect(Player* player, std::string message) {
	std::vector<uint8_t> disconnectResponse;
	player->connectionStatus = ConnectionStatus::Disconnected;
	Respond::Disconnect(disconnectResponse, player, message);
	SendToPlayer(disconnectResponse, player);
	Betrock::Server::Instance().Log(player->username + " has disconnected. (" + message + ")", LOG_INFO);
}

// Disconnects all currently connected Players
void DisconnectAllPlayers(std::string message) {
	auto &server = Betrock::Server::Instance();

	std::vector<uint8_t> disconnectResponse;
	std::scoped_lock lock(server.GetConnectedPlayerMutex());
    for (Player* player : server.GetConnectedPlayers()) {
        Disconnect(player, message);
    }
}