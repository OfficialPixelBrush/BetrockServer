#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "helper.h"
#include "player.h"

void RespondKeepAlive(std::vector<uint8_t> &response) {
    response.push_back(0);
}

void RespondLogin(std::vector<uint8_t> &response, int32_t entityId, int64_t seed, int8_t dimension) {
    response.push_back(1);
    appendIntegerToVector(response, entityId);
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

void ResponsePlayerPositionLook(std::vector<uint8_t> &response, Player* player) {
    response.push_back(0x0D);
    appendDoubleToVector(response,player->x);
    appendDoubleToVector(response,player->y);
    appendDoubleToVector(response,player->stance);
    appendDoubleToVector(response,player->z);
    appendFloatToVector(response,player->yaw);
    appendFloatToVector(response,player->pitch);
    response.push_back(player->onGround);
}

void RespondChunk(std::vector<uint8_t> &response, int32_t x, int16_t y, int32_t z, uint8_t sizeX, uint8_t sizeY, uint8_t sizeZ) {
    // , int32_t compressedSize, std::vector<uint8_t> compressedData
    response.push_back(0x33);
    appendIntegerToVector(response,x);
    appendShortToVector(response,y);
    appendIntegerToVector(response,z);
    response.push_back(sizeX);
    response.push_back(sizeY);
    response.push_back(sizeZ);
    //appendIntegerToVector(compressedSize);
    // TODO: Append actual chunk data to response vector
    appendIntegerToVector(response, 9);
    response.push_back(0x78);
    response.push_back(0xDA);
    response.push_back(0xE3);
    response.push_back(0x02);
    response.push_back(0x00);
    response.push_back(0x00);
    response.push_back(0x0B);
    response.push_back(0x00);
    response.push_back(0x0B);
}