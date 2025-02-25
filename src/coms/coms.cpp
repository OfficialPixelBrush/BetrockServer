#include "coms.h"

#include "server.h"

// Sent the specified message to all currently connected Clients
void BroadcastToClients(std::vector<uint8_t> &response, Client* sender, bool autoclear) {
	if (response.empty()) {
		return;
	}

	auto &server = Betrock::Server::Instance();

	std::scoped_lock lock(server.GetConnectedClientMutex());
    for (auto client : server.GetConnectedClients()) {
		if (client == sender) { continue; }
		if (client->GetConnectionStatus() == ConnectionStatus::Connected) {
			client->AppendResponse(response);
		}
    }
	if (autoclear) {
		response.clear();
	}
}

// Disconnects all currently connected Clients
void DisconnectAllClients(std::string message) {
	auto &server = Betrock::Server::Instance();

	std::vector<uint8_t> disconnectResponse;
	std::scoped_lock lock(server.GetConnectedClientMutex());
    for (auto client : server.GetConnectedClients()) {
        //Disconnect(player, message);
		client->HandleDisconnect(message);
    }
}