#pragma once
#include <vector>
#include <string>
#include <cstdint>

void appendShortToVector(std::vector<uint8_t> &vector, int16_t value) {
	uint8_t byte1 = (value >> 8 ) & 0xFF;
	uint8_t byte0 = (value 		) & 0xFF;
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void appendIntegerToVector(std::vector<uint8_t> &vector, int32_t value) {
	uint8_t byte3 = (value >> 24) & 0xFF;
	uint8_t byte2 = (value >> 16) & 0xFF;
	uint8_t byte1 = (value >> 8 ) & 0xFF;
	uint8_t byte0 = (value 		) & 0xFF;
	vector.push_back(byte3);
	vector.push_back(byte2);
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void appendLongToVector(std::vector<uint8_t> &vector, int64_t value) {
	uint8_t byte7 = (value >> 56) & 0xFF;
	uint8_t byte6 = (value >> 48) & 0xFF;
	uint8_t byte5 = (value >> 40) & 0xFF;
	uint8_t byte4 = (value >> 32) & 0xFF;
	uint8_t byte3 = (value >> 24) & 0xFF;
	uint8_t byte2 = (value >> 16) & 0xFF;
	uint8_t byte1 = (value >> 8 ) & 0xFF;
	uint8_t byte0 = (value 		) & 0xFF;
	vector.push_back(byte7);
	vector.push_back(byte6);
	vector.push_back(byte5);
	vector.push_back(byte4);
	vector.push_back(byte3);
	vector.push_back(byte2);
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void appendString8ToVector(std::vector<uint8_t> &vector, std::string value) {
	// Apparently strings support variable lengths, but in this case I'll always assume they're string16s
	int8_t stringLength = value.size();
	vector.push_back(stringLength 	   & 0xFF);
	for (int8_t i = 0; i < stringLength; i++) {
		vector.push_back(value[i]);
	}
}

void appendString16ToVector(std::vector<uint8_t> &vector, std::string value) {
	// Apparently strings support variable lengths, but in this case I'll always assume they're string16s
	int16_t stringLength = value.size();
	vector.push_back(stringLength >> 8 & 0xFF);
	vector.push_back(stringLength 	   & 0xFF);
	for (int16_t i = 0; i < stringLength; i++) {
		vector.push_back(value[i]);
	}
}
std::vector<std::string> packetLabels {
	"0x00 KeepAlive",
	"0x01 LoginRequest",
	"0x02 Handshake",
	"0x03 ChatMessage",
	"0x04 TimeUpdate",
	"0x05 EntityEquipment",
	"0x06 SpawnPosition",
	"0x07 UseEntity",
	"0x08 UpdateHealth",
	"0x09 Respawn",
	"0x0A Player",
	"0x0B PlayerPosition",
	"0x0C PlayerLook",
	"0x0D PlayerPositionLook",
	"0x0E PlayerDigging",
	"0x0F PlayerBlockPlacement",
	"0x10 HoldingChange",
	"0x11 UseBed",
	"0x12 Animation",
	"0x13 EntityAction???",
	"0x14 NamedEntitySpawn",
	"0x15 PickupSpawn",
	"0x16 CollectItem",
	"0x17 AddObjectVehicle",
	"0x18 MobSpawn",
	"0x19 Painting",
	"0x1A -undocumented-",
	"0x1B ???",
	"0x1C EntityVelocity?",
	"0x1D DestroyEntity",
	"0x1E Entity",
	"0x1F EntityRelativeMove",
	"0x20 EntityLook",
	"0x21 EntityLookAndRelativeMove",
	"0x22 EntityTeleport",
	"0x23 -undocumented-",
	"0x24 -undocumented-",
	"0x25 -undocumented-",
	"0x26 EntityStatus?",
	"0x27 AttachEntity?",
	"0x28 EntityMetadata",
	"0x29 -undocumented-",
	"0x2A -undocumented-",
	"0x2B -undocumented-",
	"0x2C -undocumented-",
	"0x2D -undocumented-",
	"0x2E -undocumented-",
	"0x2F -undocumented-",
	"0x30 -undocumented-",
	"0x31 -undocumented-",
	"0x32 PreChunk",
	"0x33 Chunk",
	"0x34 MultiBlockChange",
	"0x35 BlockChange",
	"0x36 PlayNoteBlock",
	"0x37 -undocumented-",
	"0x38 -undocumented-",
	"0x39 -undocumented-",
	"0x3A -undocumented-",
	"0x3B -undocumented-",
	"0x3C Explosion"
};

std::string PacketIdToLabel(uint8_t id) {
	if (id == 255) {
		return "0xFF Disconnect";
	}
	if (id > packetLabels.size()) {
		return std::to_string(id);
	}
	return packetLabels[id];
}