#include "coms.h"

#include "server.h"

// Sends the specified message to all currently connected Clients
void BroadcastToClients(std::vector<uint8_t> &response, Client* sender, bool autoclear) {
	if (response.empty()) {
		return;
	}

	auto &server = Betrock::Server::Instance();

	std::scoped_lock lock(server.GetConnectedClientMutex());
    for (auto client : server.GetConnectedClients()) {
		if (client.get() == sender) { continue; }
		if (client->GetConnectionStatus() == ConnectionStatus::Connected) {
			client->AppendResponse(response);
		}
    }
	if (autoclear) {
		response.clear();
	}
}