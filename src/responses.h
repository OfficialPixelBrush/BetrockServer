#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "helper.h"
#include "player.h"

uint8_t chunkData [] = {120,218,237,205,49,13,0,32,16,4,65,78,1,254,93,226,0,28,144,39,80,80,204,244,155,77,142,181,151,226,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,239,255,193,31,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,216,152,37,67,175,215,235,245,122,189,94,175,215,235,245,122,189,94,175,215,235,245,122,189,94,175,215,235,245,122,189,94,175,215,235,245,122,189,94,175,215,235,245,122,189,94,175,215,235,245,122,189,94,175,215,235,245,122,253,117,223,23,74,155,254,90};

void RespondKeepAlive(std::vector<uint8_t> &response) {
    response.push_back(0);
}

void RespondLogin(std::vector<uint8_t> &response, int32_t& entityId, int64_t seed, int8_t dimension) {
    response.push_back(1);
    appendIntegerToVector(response, entityId++);
    appendString16ToVector(response, "");
    appendLongToVector(response, seed);
    response.push_back(dimension);
}

// Note: Right now this just sends a "-", to tell the client that online mode is disabled
void RespondHandshake(std::vector<uint8_t> &response) {
    response.push_back(2);
    response.push_back(0);
    response.push_back(1);
    response.push_back(0);
    response.push_back('-');
}

void RespondChatMessage(std::vector<uint8_t> &response, std::string message) {
    response.push_back(0x03);
    appendString16ToVector(response,message);
}

void RespondTime(std::vector<uint8_t> &response, int64_t time) {
    response.push_back(4);
    appendLongToVector(response, time);
}

void RespondSpawnPoint(std::vector<uint8_t> &response, int32_t x, int32_t y, int32_t z) {
    response.push_back(6);
    appendIntegerToVector(response, x);
    appendIntegerToVector(response, y);
    appendIntegerToVector(response, z);
}

void RespondUpdateHealth(std::vector<uint8_t> &response, int16_t health) {
    response.push_back(8);
    appendShortToVector(response, health);
}

void RespondPlayerPosition(std::vector<uint8_t> &response, Player* player) {
    response.push_back(0x0B);
    appendDoubleToVector(response,player->x);
    appendDoubleToVector(response,player->y);
    appendDoubleToVector(response,player->stance);
    appendDoubleToVector(response,player->z);
    response.push_back(player->onGround);
}

void RespondPlayerPositionLook(std::vector<uint8_t> &response, Player* player) {
    response.push_back(0x0D);
    appendDoubleToVector(response,player->x);
    appendDoubleToVector(response,player->y);
    appendDoubleToVector(response,player->stance);
    appendDoubleToVector(response,player->z);
    appendFloatToVector(response,player->yaw);
    appendFloatToVector(response,player->pitch);
    response.push_back(player->onGround);
}

void RespondPlayerDigging(std::vector<uint8_t> &response, int8_t status, int32_t x, int8_t y, int32_t z, int8_t face) {
    response.push_back(0x0E);
    response.push_back(status);
    appendIntegerToVector(response, x);
    response.push_back(y);
    appendIntegerToVector(response, z);
    response.push_back(face);
}

void RespondPlayerBlockPlacement(std::vector<uint8_t> &response, int32_t x, int8_t y, int32_t z, int8_t direction, int16_t id, int8_t amount, int16_t damage) {
    response.push_back(0x0F);
    appendIntegerToVector(response, x);
    response.push_back(y);
    appendIntegerToVector(response, z);
    response.push_back(direction);
    appendShortToVector(response, id);
    response.push_back(amount);
    appendShortToVector(response, damage);
}

void RespondAnimation(std::vector<uint8_t> &response, int32_t entityId, uint8_t animation) {
    response.push_back(0x12);
    appendIntegerToVector(response, entityId);
    response.push_back(animation);
}

void RespondNamedEntitySpawn(std::vector<uint8_t> &response, int32_t& entityId, std::string username, int32_t x, int32_t y, int32_t z, int8_t yaw, int8_t pitch, int16_t currentItem) {
    response.push_back(0x14);
    appendIntegerToVector(response, entityId++);
    appendString16ToVector(response, username);
    appendIntegerToVector(response, x);
    appendIntegerToVector(response, y);
    appendIntegerToVector(response, z);
    response.push_back(yaw);
    response.push_back(pitch);
    appendShortToVector(response, currentItem);
}

void RespondMobSpawn(std::vector<uint8_t> &response, int32_t& entityId, int8_t type, int32_t x, int32_t y, int32_t z, int8_t yaw, int8_t pitch) {
    response.push_back(0x18);
    appendIntegerToVector(response, entityId++);
    response.push_back(type);
    appendIntegerToVector(response, x);
    appendIntegerToVector(response, y);
    appendIntegerToVector(response, z);
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

void RespondPreChunk(std::vector<uint8_t> &response, int32_t x, int32_t z, bool mode) {
    // , int32_t compressedSize, std::vector<uint8_t> compressedData
    response.push_back(0x32);
    appendIntegerToVector(response,x);
    appendIntegerToVector(response,z);
    response.push_back(mode);
}

void RespondChunk(std::vector<uint8_t> &response, int32_t x, int16_t y, int32_t z, uint8_t sizeX, uint8_t sizeY, uint8_t sizeZ, size_t compressedSize, char* compressedData) {
    if (compressedSize == 0 || compressedData == nullptr) { return; }
    response.push_back(0x33);
    appendIntegerToVector(response,x);
    appendShortToVector(response,y);
    appendIntegerToVector(response,z);
    response.push_back(sizeX);
    response.push_back(sizeY);
    response.push_back(sizeZ);
    appendIntegerToVector(response,compressedSize);
    for (int i = 0; i < compressedSize; i++) {
        response.push_back(compressedData[i]);
    }
}

void RespondTempChunk(std::vector<uint8_t> &response, int32_t x, int16_t y, int32_t z) {
    RespondChunk(response,x,y,z,15,127,15,sizeof(chunkData),(char*)chunkData);
}

void RespondBlockChange(std::vector<uint8_t> &response, int32_t x, int16_t y, int32_t z, int8_t type, int8_t meta) {
    response.push_back(0x35);
    appendIntegerToVector(response,x);
    response.push_back(y);
    appendIntegerToVector(response,z);
    response.push_back(type);
    response.push_back(meta);
}

void RespondSetSlot(std::vector<uint8_t> &response, int8_t windowId, int16_t slot, int16_t itemId, int8_t itemCount, int16_t itemUses) {
    response.push_back(0x67);
    response.push_back(windowId); // Player Inventory
    appendShortToVector(response,slot);
    appendShortToVector(response,itemId);
    response.push_back(itemCount); // Player Inventory
    appendShortToVector(response,itemUses);
}

void RespondWindowItems(std::vector<uint8_t> &response, int8_t windowId, int16_t count) {
    response.push_back(0x68);
    response.push_back(windowId); // Player Inventory
    appendShortToVector(response, count);
    for (int i = 0; i < 35; i++) {
        appendShortToVector(response, 277);
    }
}