#include "responses.h"

void Respond::KeepAlive(std::vector<uint8_t> &response) {
    response.push_back(0);
}

void Respond::Login(std::vector<uint8_t> &response, int32_t& entityId, int64_t seed, int8_t dimension) {
    response.push_back(1);
    AppendIntegerToVector(response, entityId);
    AppendString16ToVector(response, "");
    AppendLongToVector(response, seed);
    response.push_back(dimension);
}

// Note: Right now this just sends a "-", to tell the client that online mode is disabled
void Respond::Handshake(std::vector<uint8_t> &response) {
    response.push_back(2);
    AppendString16ToVector(response,"-");
}

void Respond::ChatMessage(std::vector<uint8_t> &response, std::string message, bool toConsole) {
    response.push_back(0x03);
    AppendString16ToVector(response,message);
    if (toConsole) {
        std::cout << message << std::endl;
    }
}

void Respond::Time(std::vector<uint8_t> &response, int64_t time) {
    response.push_back(4);
    AppendLongToVector(response, time);
}

void Respond::SpawnPoint(std::vector<uint8_t> &response, Int3 position) {
    response.push_back(6);
    AppendIntegerToVector(response, position.x);
    AppendIntegerToVector(response, position.y);
    AppendIntegerToVector(response, position.z);
}

void Respond::UpdateHealth(std::vector<uint8_t> &response, int16_t health) {
    response.push_back(8);
    AppendShortToVector(response, health);
}

void Respond::Respawn(std::vector<uint8_t> &response, int8_t dimension) {
    response.push_back(0x09);
    response.push_back(dimension);
}

void Respond::PlayerPosition(std::vector<uint8_t> &response, Player* player) {
    response.push_back(0x0B);
    AppendDoubleToVector(response,player->position.x);
    AppendDoubleToVector(response,player->position.y);
    AppendDoubleToVector(response,player->stance);
    AppendDoubleToVector(response,player->position.z);
    response.push_back(player->onGround);
}

void Respond::PlayerPositionLook(std::vector<uint8_t> &response, Player* player) {
    response.push_back(0x0D);
    AppendDoubleToVector(response,player->position.x);
    AppendDoubleToVector(response,player->stance);
    AppendDoubleToVector(response,player->position.y);
    AppendDoubleToVector(response,player->position.z);
    AppendFloatToVector(response,player->yaw);
    AppendFloatToVector(response,player->pitch);
    response.push_back(player->onGround);
}

void Respond::PlayerDigging(std::vector<uint8_t> &response, int8_t status, Int3 position, int8_t face) {
    response.push_back(0x0E);
    response.push_back(status);
    AppendIntegerToVector(response, position.x);
    response.push_back((int8_t)position.y);
    AppendIntegerToVector(response, position.z);
    response.push_back(face);
}

void Respond::PlayerBlockPlacement(std::vector<uint8_t> &response, Int3 position, int8_t direction, int16_t id, int8_t amount, int16_t damage) {
    response.push_back(0x0F);
    AppendIntegerToVector(response, position.x);
    response.push_back((int8_t)position.y);
    AppendIntegerToVector(response, position.z);
    response.push_back(direction);
    AppendShortToVector(response, id);
    response.push_back(amount);
    AppendShortToVector(response, damage);
}

void Respond::Animation(std::vector<uint8_t> &response, int32_t entityId, uint8_t animation) {
    response.push_back(0x12);
    AppendIntegerToVector(response, entityId);
    response.push_back(animation);
}

void Respond::NamedEntitySpawn(std::vector<uint8_t> &response, int32_t& entityId, std::string username, Int3 position, int8_t yaw, int8_t pitch, int16_t currentItem) {
    response.push_back(0x14);
    AppendIntegerToVector(response, entityId);
    AppendString16ToVector(response, username);
    AppendIntegerToVector(response, position.x);
    AppendIntegerToVector(response, position.y);
    AppendIntegerToVector(response, position.z);
    response.push_back(yaw);
    response.push_back(pitch);
    AppendShortToVector(response, currentItem);
}

void Respond::MobSpawn(std::vector<uint8_t> &response, int32_t& entityId, int8_t type, Int3 position, int8_t yaw, int8_t pitch) {
    response.push_back(0x18);
    AppendIntegerToVector(response, entityId);
    response.push_back(type);
    AppendIntegerToVector(response, position.x);
    AppendIntegerToVector(response, position.y);
    AppendIntegerToVector(response, position.z);
    response.push_back(yaw);
    response.push_back(pitch);
    response.push_back(127);
    /*
    int8_t data = 0;
    size_t i = 0;
    while (data != 127) {
        // TODO: Implement writing of metadata
        data = metadata[i];
    }
    */
}

void Respond::PreChunk(std::vector<uint8_t> &response, int32_t x, int32_t z, bool mode) {
    // , int32_t compressedSize, std::vector<uint8_t> compressedData
    response.push_back(0x32);
    AppendIntegerToVector(response,x);
    AppendIntegerToVector(response,z);
    response.push_back(mode);
}

void Respond::Chunk(std::vector<uint8_t> &response, Int3 position, uint8_t sizeX, uint8_t sizeY, uint8_t sizeZ, size_t compressedSize, char* compressedData) {
    if (compressedSize == 0 || compressedData == nullptr) { return; }
    response.push_back(0x33);
    AppendIntegerToVector(response,position.x);
    AppendShortToVector(response,(int16_t)position.y);
    AppendIntegerToVector(response,position.z);
    response.push_back(sizeX);
    response.push_back(sizeY);
    response.push_back(sizeZ);
    AppendIntegerToVector(response,compressedSize);
    response.insert(response.end(), compressedData, compressedData + compressedSize);
}

void Respond::BlockChange(std::vector<uint8_t> &response, Int3 position, int8_t type, int8_t meta) {
    response.push_back(0x35);
    AppendIntegerToVector(response,position.x);
    response.push_back((int8_t)position.y);
    AppendIntegerToVector(response,position.z);
    response.push_back(type);
    response.push_back(meta);
}

void Respond::SetSlot(std::vector<uint8_t> &response, int8_t windowId, int16_t slot, int16_t itemId, int8_t itemCount, int16_t itemUses) {
    response.push_back(0x67);
    response.push_back(windowId); // Player Inventory
    AppendShortToVector(response,slot);
    AppendShortToVector(response,itemId);
    response.push_back(itemCount); // Player Inventory
    AppendShortToVector(response,itemUses);
}

void Respond::WindowItems(std::vector<uint8_t> &response, int8_t windowId, int16_t count) {
    response.push_back(0x68);
    response.push_back(windowId); // Player Inventory
    AppendShortToVector(response, count);
    for (int i = 0; i < 35; i++) {
        AppendShortToVector(response, 277);
    }
}

void Respond::Disconnect(std::vector<uint8_t> &response, Player* player, std::string message) {
	std::vector<uint8_t> disconnectResponse;
	std::cout << player->username << " has disconnected. (" << message << ")" << std::endl;
	disconnectResponse.push_back(0xFF);
	AppendString16ToVector(disconnectResponse,message);
}