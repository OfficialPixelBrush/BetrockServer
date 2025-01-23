#include "coms.h"

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
	std::lock_guard<std::mutex> lock(connectedPlayersMutex);
    for (Player *player : connectedPlayers) {
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
	std::cout << player->username << " has disconnected. (" << message << ")" << std::endl;
}

// Disconnects all currently connected Players
void DisconnectAllPlayers(std::string message) {
	std::vector<uint8_t> disconnectResponse;
	std::lock_guard<std::mutex> lock(connectedPlayersMutex);
    for (Player* player : connectedPlayers) {
        Disconnect(player, message);
    }
}

size_t SendChunksAroundPlayer(std::vector<uint8_t> &response, Player* player) {
	size_t numberOfNewChunks = 0;
	
	// Determine the center chunk from which to start from
	Int3 centerPos = Vec3ToInt3(player->position);
	int32_t pX = centerPos.x >> 4;
	int32_t pZ = centerPos.z >> 4;

	// Go over all chunk coordinates around the center position
	for (int x = pX + chunkDistance*-1; x < pX + chunkDistance; x++) {
		for (int z = pZ + chunkDistance*-1; z < pZ + chunkDistance; z++) {
			World* world = GetDimension(player->dimension);
			Int3 position = XyzToInt3(x,0,z);

			// Acquire existing chunk data
			std::vector<uint8_t> chunkData = world->GetChunkData(position);
			if (chunkData.empty()) {
				// If none exists, generate new chunks
				world->GenerateChunk(x,z);
				numberOfNewChunks++;
				chunkData = world->GetChunkData(position);
			}
			// Then compress that chunk data and send it to the client
			if (!chunkData.empty()) {
				size_t compressedSize = 0;
				char* chunk = CompressChunk(chunkData,compressedSize);
				if (chunk) {
					// The Client 100% expects to get a PreChunk before a Chunk,
					// at least when joining, otherwise it implodes
					Respond::PreChunk(response,x,z,1);
					Respond::Chunk(response,XyzToInt3(x*16,0,z*16),15,127,15,compressedSize,chunk);
				}
				delete[] chunk;
			} else {
				std::cout << "Failed to generated Chunk (" << (int)x << ", " << (int)z << ")" << std::endl;
			}
		}
	}
	return numberOfNewChunks;
}