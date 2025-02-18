#include "responses.h"
#include "player.h"
#include "entity.h"
#include <cstdint>

void Respond::KeepAlive(std::vector<uint8_t> &response) {
    response.push_back((uint8_t)Packet::KeepAlive);
}

void Respond::Login(std::vector<uint8_t> &response, int32_t& entityId, int64_t seed, int8_t dimension) {
    response.push_back((uint8_t)Packet::LoginRequest);
    AppendIntegerToVector(response, entityId);
    AppendString16ToVector(response, "");
    AppendLongToVector(response, seed);
    response.push_back(dimension);
}

// Note: Right now this just sends a "-", to tell the client that online mode is disabled
void Respond::Handshake(std::vector<uint8_t> &response) {
    response.push_back((uint8_t)Packet::Handshake);
    AppendString16ToVector(response,"-");
}

void Respond::ChatMessage(std::vector<uint8_t> &response, std::string message, bool toConsole) {
    response.push_back((uint8_t)Packet::ChatMessage);
    AppendString16ToVector(response,message);
    if (toConsole) {
        Betrock::Logger::Instance().ChatMessage(message);
    }
}

void Respond::Time(std::vector<uint8_t> &response, int64_t time) {
    response.push_back((uint8_t)Packet::TimeUpdate);
    AppendLongToVector(response, time);
}

void Respond::EntityEquipment(std::vector<uint8_t> &response, int32_t entityId, int16_t slotId, int16_t item, int16_t damage) {
    response.push_back((uint8_t)Packet::EntityEquipment);
    AppendIntegerToVector(response, entityId);
    AppendShortToVector(response, slotId);
    AppendShortToVector(response, item);
    AppendShortToVector(response, damage);
}

void Respond::SpawnPoint(std::vector<uint8_t> &response, Int3 position) {
    response.push_back((uint8_t)Packet::SpawnPosition);
    AppendIntegerToVector(response, position.x);
    AppendIntegerToVector(response, position.y);
    AppendIntegerToVector(response, position.z);
}

void Respond::UpdateHealth(std::vector<uint8_t> &response, int16_t health) {
    response.push_back((uint8_t)Packet::UpdateHealth);
    AppendShortToVector(response, health);
}

void Respond::Respawn(std::vector<uint8_t> &response, int8_t dimension) {
    response.push_back((uint8_t)Packet::Respawn);
    response.push_back(dimension);
}

void Respond::PlayerPosition(std::vector<uint8_t> &response, Player* player) {
    response.push_back((uint8_t)Packet::PlayerPosition);
    AppendDoubleToVector(response,player->position.x);
    AppendDoubleToVector(response,player->position.y);
    AppendDoubleToVector(response,player->stance);
    AppendDoubleToVector(response,player->position.z);
    response.push_back(player->onGround);
}

void Respond::PlayerPositionLook(std::vector<uint8_t> &response, Player* player) {
    response.push_back((uint8_t)Packet::PlayerPositionLook);
    AppendDoubleToVector(response,player->position.x);
    AppendDoubleToVector(response,player->stance);
    AppendDoubleToVector(response,player->position.y);
    AppendDoubleToVector(response,player->position.z);
    AppendFloatToVector(response,player->yaw);
    AppendFloatToVector(response,player->pitch);
    response.push_back(player->onGround);
}

void Respond::PlayerDigging(std::vector<uint8_t> &response, int8_t status, Int3 position, int8_t face) {
    response.push_back((uint8_t)Packet::PlayerDigging);
    response.push_back(status);
    AppendIntegerToVector(response, position.x);
    response.push_back((int8_t)position.y);
    AppendIntegerToVector(response, position.z);
    response.push_back(face);
}

void Respond::PlayerBlockPlacement(std::vector<uint8_t> &response, Int3 position, int8_t direction, int16_t id, int8_t amount, int16_t damage) {
    response.push_back((uint8_t)Packet::PlayerBlockPlacement);
    AppendIntegerToVector(response, position.x);
    response.push_back((int8_t)position.y);
    AppendIntegerToVector(response, position.z);
    response.push_back(direction);
    AppendShortToVector(response, id);
    response.push_back(amount);
    AppendShortToVector(response, damage);
}

void Respond::Animation(std::vector<uint8_t> &response, int32_t entityId, uint8_t animation) {
    response.push_back((uint8_t)Packet::Animation);
    AppendIntegerToVector(response, entityId);
    response.push_back(animation);
}

void Respond::EntityAction(std::vector<uint8_t> &response, int32_t entityId, uint8_t action) {
    response.push_back((uint8_t)Packet::EntityAction);
    AppendIntegerToVector(response, entityId);
    response.push_back(action);
}

void Respond::NamedEntitySpawn(std::vector<uint8_t> &response, int32_t& entityId, std::string username, Int3 position, int8_t yaw, int8_t pitch, int16_t currentItem) {
    response.push_back((uint8_t)Packet::NamedEntitySpawn);
    AppendIntegerToVector(response, entityId);
    AppendString16ToVector(response, username);
    AppendIntegerToVector(response, position.x);
    AppendIntegerToVector(response, position.y);
    AppendIntegerToVector(response, position.z);
    response.push_back(yaw);
    response.push_back(pitch);
    AppendShortToVector(response, currentItem);
}

void Respond::PickupSpawn(std::vector<uint8_t> &response, int32_t& entityId, int16_t id, int8_t count, int16_t damage, Int3 position, int8_t yaw, int8_t pitch, int8_t roll) {
    response.push_back((uint8_t)Packet::PickupSpawn);
    AppendIntegerToVector(response, entityId++);
    AppendShortToVector(response, id);
    response.push_back(count);
    AppendShortToVector(response, damage);
    AppendIntegerToVector(response, position.x);
    AppendIntegerToVector(response, position.y);
    AppendIntegerToVector(response, position.z);
    response.push_back(yaw);
    response.push_back(pitch);
    response.push_back(roll);
}

void Respond::MobSpawn(std::vector<uint8_t> &response, int32_t& entityId, int8_t type, Int3 position, int8_t yaw, int8_t pitch) {
    response.push_back((uint8_t)Packet::MobSpawn);
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

void Respond::DestroyEntity(std::vector<uint8_t> &response, int32_t& entityId) {
    response.push_back((uint8_t)Packet::DestroyEntity);
    AppendIntegerToVector(response, entityId);
}

void Respond::EntityRelativeMove(std::vector<uint8_t> &response, int32_t& entityId, Int3 relativeMovement) {
    response.push_back((uint8_t)Packet::EntityRelativeMove);
    AppendIntegerToVector(response, entityId);
    response.push_back(relativeMovement.x);
    response.push_back(relativeMovement.y);
    response.push_back(relativeMovement.z);
}

void Respond::EntityLook(std::vector<uint8_t> &response, int32_t& entityId, int8_t yaw, int8_t pitch) {
    response.push_back((uint8_t)Packet::EntityLook);
    AppendIntegerToVector(response, entityId);
    response.push_back(yaw);
    response.push_back(pitch);
}

void Respond::EntityLookRelativeMove(std::vector<uint8_t> &response, int32_t& entityId, Int3 relativeMovement, int8_t yaw, int8_t pitch) {
    response.push_back((uint8_t)Packet::EntityLookRelativeMove);
    AppendIntegerToVector(response, entityId);
    response.push_back(relativeMovement.x);
    response.push_back(relativeMovement.y);
    response.push_back(relativeMovement.z);
    response.push_back(yaw);
    response.push_back(pitch);
}

void Respond::EntityTeleport(std::vector<uint8_t> &response, int32_t& entityId, Int3 position, int8_t yaw, int8_t pitch) {
    response.push_back((uint8_t)Packet::EntityTeleport);
    AppendIntegerToVector(response, entityId);
    AppendIntegerToVector(response, position.x);
    AppendIntegerToVector(response, position.y);
    AppendIntegerToVector(response, position.z);
    response.push_back(yaw);
    response.push_back(pitch);
}

void Respond::EntityMetadata(std::vector<uint8_t> &response, int32_t& entityId, int8_t byte) {
    response.push_back((uint8_t)Packet::EntityMetadata);
    AppendIntegerToVector(response, entityId);
    response.push_back(0);
    response.push_back(byte);
    response.push_back(127);
}

void Respond::PreChunk(std::vector<uint8_t> &response, int32_t x, int32_t z, bool mode) {
    // , int32_t compressedSize, std::vector<uint8_t> compressedData
    response.push_back((uint8_t)Packet::PreChunk);
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
    response.push_back((uint8_t)Packet::BlockChange);
    AppendIntegerToVector(response,position.x);
    response.push_back((int8_t)position.y);
    AppendIntegerToVector(response,position.z);
    response.push_back(type);
    response.push_back(meta);
}

void Respond::Soundeffect(std::vector<uint8_t> &response, int32_t sound, Int3 position, int32_t extra) {
    response.push_back((uint8_t)Packet::Soundeffect);
    AppendIntegerToVector(response,sound);
    AppendIntegerToVector(response,position.x);
    response.push_back((int8_t)position.y);
    AppendIntegerToVector(response,position.z);
    AppendIntegerToVector(response,extra);
}

void Respond::SetSlot(std::vector<uint8_t> &response, int8_t window, int16_t slot, int16_t item, int8_t amount, int16_t damage) {
    response.push_back((uint8_t)Packet::SetSlot);
    response.push_back(window);
    AppendShortToVector(response,slot);
    AppendShortToVector(response,item);
    response.push_back(amount);
    AppendShortToVector(response,damage);
}

void Respond::WindowItems(std::vector<uint8_t> &response, int8_t window, std::vector<Item> payload) {
    response.push_back((uint8_t)Packet::WindowItems);
    response.push_back(window); // Player Inventory
    AppendShortToVector(response, payload.size());
    for (int16_t slot = 0; slot < payload.size(); slot++) {
        Item i = payload[slot];
        AppendShortToVector(response,i.id);
        if (i.id > SLOT_EMPTY) {
            response.push_back(i.amount); // Player Inventory
            AppendShortToVector(response,i.damage);
        }
    }
}

void Respond::Disconnect(std::vector<uint8_t> &response, Player* player, std::string message) {
	std::vector<uint8_t> disconnectResponse;
	disconnectResponse.push_back((uint8_t)Packet::Disconnect);
	AppendString16ToVector(disconnectResponse,message);
}