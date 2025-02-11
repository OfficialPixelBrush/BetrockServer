#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "helper.h"
#include "inventory.h"
#include "items.h"

class Entity; // Forward declaration
class Player; // Forward declaration

class Respond {
    public:
        static void KeepAlive(std::vector<uint8_t> &response);
        static void Login(std::vector<uint8_t> &response, int32_t& entityId, int64_t seed, int8_t dimension);
        static void Handshake(std::vector<uint8_t> &response);
        static void ChatMessage(std::vector<uint8_t> &response, std::string message, bool toConsole = 1);
        static void Time(std::vector<uint8_t> &response, int64_t time);
        static void EntityEquipment(std::vector<uint8_t> &response, int32_t entityId, int16_t slotId, int16_t itemId, int16_t damage);
        static void SpawnPoint(std::vector<uint8_t> &response, Int3 position);
        static void UpdateHealth(std::vector<uint8_t> &response, int16_t health);
        static void Respawn(std::vector<uint8_t> &response, int8_t dimension);
        static void PlayerPosition(std::vector<uint8_t> &response, Player* player);
        static void PlayerPositionLook(std::vector<uint8_t> &response, Player* player);
        static void PlayerDigging(std::vector<uint8_t> &response, int8_t status, Int3 position, int8_t face);
        static void PlayerBlockPlacement(std::vector<uint8_t> &response, Int3 position, int8_t direction, int16_t id, int8_t amount, int16_t damage);
        static void Animation(std::vector<uint8_t> &response, int32_t entityId, uint8_t animation);
        static void EntityAction(std::vector<uint8_t> &response, int32_t entityId, uint8_t action);
        static void NamedEntitySpawn(std::vector<uint8_t> &response, int32_t& entityId, std::string username, Int3 position, int8_t yaw, int8_t pitch, int16_t currentItem);
        static void PickupSpawn(std::vector<uint8_t> &response, int32_t& entityId, int16_t id, int8_t count, int16_t damage, Int3 position, int8_t yaw, int8_t pitch, int8_t roll);
        static void MobSpawn(std::vector<uint8_t> &response, int32_t& entityId, int8_t type, Int3 position, int8_t yaw, int8_t pitch);
        static void DestroyEntity(std::vector<uint8_t> &response, int32_t& entityId);
        static void EntityRelativeMove(std::vector<uint8_t> &response, int32_t& entityId, Int3 relativeMovement);
        static void EntityLook(std::vector<uint8_t> &response, int32_t& entityId, int8_t yaw, int8_t pitch);
        static void EntityLookRelativeMove(std::vector<uint8_t> &response, int32_t& entityId, Int3 relativeMovement, int8_t yaw, int8_t pitch);
        static void EntityTeleport(std::vector<uint8_t> &response, int32_t& entityId, Int3 position, int8_t yaw, int8_t pitch);
        static void EntityStatus(std::vector<uint8_t> &response, int32_t& entityId, int8_t status);
        static void PreChunk(std::vector<uint8_t> &response, int32_t x, int32_t z, bool mode);
        static void Chunk(std::vector<uint8_t> &response, Int3 position, uint8_t sizeX, uint8_t sizeY, uint8_t sizeZ, size_t compressedSize, char* compressedData);
        static void BlockChange(std::vector<uint8_t> &response, Int3 position, int8_t type, int8_t meta);
        static void Soundeffect(std::vector<uint8_t> &response, int32_t sound, Int3 position, int32_t extra);
        static void SetSlot(std::vector<uint8_t> &response, int8_t windowId, int16_t slot, int16_t itemId, int8_t itemCount, int16_t itemUses);
        static void WindowItems(std::vector<uint8_t> &response, int8_t windowId, std::vector<Item> payload);
        static void Disconnect(std::vector<uint8_t> &response, Player* player, std::string message);
};