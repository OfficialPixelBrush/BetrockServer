#include "coms.h"

#include "server.h"
#include <cstdint>
#include <sys/types.h>

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

// --- Reading of packet data ---
int8_t EntryToByte(uint8_t* message, size_t& offset) {
	int8_t result = int8_t(message[offset]);
	offset++;
	return result;
}

int16_t EntryToShort(uint8_t* message, size_t& offset) {
	int16_t result = (
		int16_t(message[offset  ]) << 8 |
		int16_t(message[offset+1])
	);
	offset+=2;
	return result;
}

int32_t EntryToInteger(uint8_t* message, size_t& offset) {
	int32_t result = (
		int32_t(message[offset  ]) << 24 |
		int32_t(message[offset+1]) << 16 |
		int32_t(message[offset+2]) <<  8 |
		int32_t(message[offset+3])
	);
	offset+=4;
	return result;
}

int64_t EntryToLong(uint8_t* message, size_t& offset) {
	int64_t result = (
		int64_t(message[offset  ]) << 56 |
		int64_t(message[offset+1]) << 48 |
		int64_t(message[offset+2]) << 40 |
		int64_t(message[offset+3]) << 32 |
		int64_t(message[offset+4]) << 24 |
		int64_t(message[offset+5]) << 16 |
		int64_t(message[offset+6]) <<  8 |
		int64_t(message[offset+7])
	);
	offset+=8;
	return result;
}

float EntryToFloat(uint8_t* message, size_t& offset) {
    uint32_t intBits = (
		(static_cast<uint32_t>(message[offset    ]) << 24) |
		(static_cast<uint32_t>(message[offset + 1]) << 16) |
		(static_cast<uint32_t>(message[offset + 2]) <<  8) |
		(static_cast<uint32_t>(message[offset + 3])      )
	);
    float result;
    std::memcpy(&result, &intBits, sizeof(result)); // Copy bits into a float
    offset += 4;
    return result;
}

double EntryToDouble(uint8_t* message, size_t& offset) {
    uint64_t intBits = (
		(static_cast<uint64_t>(message[offset    ]) << 56) |
		(static_cast<uint64_t>(message[offset + 1]) << 48) |
		(static_cast<uint64_t>(message[offset + 2]) << 40) |
		(static_cast<uint64_t>(message[offset + 3]) << 32) |
		(static_cast<uint64_t>(message[offset + 4]) << 24) |
		(static_cast<uint64_t>(message[offset + 5]) << 16) |
		(static_cast<uint64_t>(message[offset + 6]) <<  8) |
		(static_cast<uint64_t>(message[offset + 7])      )
	);
    double result;
    std::memcpy(&result, &intBits, sizeof(result)); // Copy bits into a double
    offset += 8; // Correctly increment offset by 8 for double
    return result;
}

// Turns out this is just UTF-8, not Strings with an 8-Bit Length
std::string EntryToString8(uint8_t* message, size_t& offset) {
	std::string string8 = "";
	size_t stringSize = size_t(EntryToShort(message, offset));
	for (size_t i = 0; i < stringSize; i++) {
		string8 += char(message[i + offset]);
	}
	offset += stringSize;
	return string8;
}

// Turns out this is just UTF-16, not Strings with a 16-Bit Length
std::string EntryToString16(uint8_t* message, size_t& offset) {
	std::string string16 = "";
	size_t stringSize = size_t(EntryToShort(message, offset))*2;
	for (size_t i = 0; i < stringSize; i+=2) {
		string16 += char(message[i+offset] << 8 | message[i+offset+1]);
	}
	offset += stringSize;
	return string16;
}

// --- Sending of packet data ---
void AppendShortToVector(std::vector<uint8_t> &vector, int16_t value) {
	uint8_t byte1 = uint8_t((value >> 8 ) & 0xFF);
	uint8_t byte0 = uint8_t((value 		) & 0xFF);
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void AppendIntegerToVector(std::vector<uint8_t> &vector, int32_t value) {
	uint8_t byte3 = uint8_t((value >> 24) & 0xFF);
	uint8_t byte2 = uint8_t((value >> 16) & 0xFF);
	uint8_t byte1 = uint8_t((value >> 8 ) & 0xFF);
	uint8_t byte0 = uint8_t((value 		) & 0xFF);
	vector.push_back(byte3);
	vector.push_back(byte2);
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void AppendLongToVector(std::vector<uint8_t> &vector, int64_t value) {
	uint8_t byte7 = uint8_t((value >> 56) & 0xFF);
	uint8_t byte6 = uint8_t((value >> 48) & 0xFF);
	uint8_t byte5 = uint8_t((value >> 40) & 0xFF);
	uint8_t byte4 = uint8_t((value >> 32) & 0xFF);
	uint8_t byte3 = uint8_t((value >> 24) & 0xFF);
	uint8_t byte2 = uint8_t((value >> 16) & 0xFF);
	uint8_t byte1 = uint8_t((value >> 8 ) & 0xFF);
	uint8_t byte0 = uint8_t((value      ) & 0xFF);
	vector.push_back(byte7);
	vector.push_back(byte6);
	vector.push_back(byte5);
	vector.push_back(byte4);
	vector.push_back(byte3);
	vector.push_back(byte2);
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void AppendFloatToVector(std::vector<uint8_t> &vector, float value) {
    uint32_t intValue;
    std::memcpy(&intValue, &value, sizeof(float));
	uint8_t byte3 = uint8_t((intValue >> 24) & 0xFF);
	uint8_t byte2 = uint8_t((intValue >> 16) & 0xFF);
	uint8_t byte1 = uint8_t((intValue >> 8 ) & 0xFF);
	uint8_t byte0 = uint8_t((intValue      ) & 0xFF);
	vector.push_back(byte3);
	vector.push_back(byte2);
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void AppendDoubleToVector(std::vector<uint8_t> &vector, double value) {
    uint64_t intValue;
    std::memcpy(&intValue, &value, sizeof(double));
	uint8_t byte7 = uint8_t((intValue >> 56) & 0xFF);
	uint8_t byte6 = uint8_t((intValue >> 48) & 0xFF);
	uint8_t byte5 = uint8_t((intValue >> 40) & 0xFF);
	uint8_t byte4 = uint8_t((intValue >> 32) & 0xFF);
	uint8_t byte3 = uint8_t((intValue >> 24) & 0xFF);
	uint8_t byte2 = uint8_t((intValue >> 16) & 0xFF);
	uint8_t byte1 = uint8_t((intValue >> 8 ) & 0xFF);
	uint8_t byte0 = uint8_t((intValue      ) & 0xFF);
	vector.push_back(byte7);
	vector.push_back(byte6);
	vector.push_back(byte5);
	vector.push_back(byte4);
	vector.push_back(byte3);
	vector.push_back(byte2);
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void AppendString8ToVector(std::vector<uint8_t> &vector, std::string value) {
	// Apparently strings support variable lengths, but in this case I'll always assume they're string16s
	AppendShortToVector(vector, int16_t(value.size()));
	for (size_t i = 0; i < value.size(); i++) {
		vector.push_back(uint8_t(value[i]));
	}
}

void AppendString16ToVector(std::vector<uint8_t> &vector, std::string value) {
	// Apparently strings support variable lengths, but in this case I'll always assume they're string16s
	AppendShortToVector(vector, int16_t(value.size()));
	for (size_t i = 0; i < value.size(); i++) {
		vector.push_back(0);
		vector.push_back(uint8_t(value[i]));
	}
}