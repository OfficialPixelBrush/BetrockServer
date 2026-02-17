#pragma once
#include <iostream>
#include <cstdint>
#include <vector>
#include <mutex>

#include "platform.h"

#include "client.h"
#include "debug.h"
#include "responses.h"
#include "world.h"
#include "packets.h"

class Client;

void BroadcastToClients(std::vector<uint8_t> &response, Client* sender = nullptr, bool autoclear = true);

// Converting a network response into a native type
int8_t EntryToByte(uint8_t* message, size_t& offset);
int16_t EntryToShort(uint8_t* message, size_t& offset);
int32_t EntryToInteger(uint8_t* message, size_t& offset);
int64_t EntryToLong(uint8_t* message, size_t& offset);
float EntryToFloat(uint8_t* message, size_t& offset);
double EntryToDouble(uint8_t* message, size_t& offset);
std::string EntryToString8(uint8_t* message, size_t& offset);
std::string EntryToString16(uint8_t* message, size_t& offset);

// Appending Data onto Network Response
void AppendShortToVector(std::vector<uint8_t> &vector, int16_t value);
void AppendIntegerToVector(std::vector<uint8_t> &vector, int32_t value);
void AppendLongToVector(std::vector<uint8_t> &vector, int64_t value);
void AppendFloatToVector(std::vector<uint8_t> &vector, float value);
void AppendDoubleToVector(std::vector<uint8_t> &vector, double value);
void AppendString8ToVector(std::vector<uint8_t> &vector, std::string value);
void AppendString16ToVector(std::vector<uint8_t> &vector, std::string value);