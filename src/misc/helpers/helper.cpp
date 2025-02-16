#include "helper.h"

Vec3 Int3ToVec3(Int3 i) {
	Vec3 v = {
		(double)i.x,
		(double)i.y,
		(double)i.z
	};
	return v;
}

Int3 Vec3ToInt3(Vec3 v) {
	Int3 i = {
		(int32_t)v.x,
		(int32_t)v.y,
		(int32_t)v.z
	};
	return i;
}

Int3 XyzToInt3(int32_t x, int32_t y, int32_t z) {
	Int3 i = {
		x,
		y,
		z
	};
	return i;
}

bool Between(int value, int a, int b) {
	if (a < b) {
		if (value > a && value < b) {
			return true;
		}
	} else {
		if (value > b && value < a) {
			return true;
		}
	}
	return false;
}

double GetDistance(Vec3 a, Vec3 b) {
	double x = (b.x-a.x)*(b.x-a.x);
	double y = (b.y-a.y)*(b.y-a.y);
	double z = (b.z-a.z)*(b.z-a.z);
	return abs(std::sqrt(x+y+z));
}

double GetDistance(Int3 a, Int3 b) {
	int32_t x = (b.x-a.x)*(b.x-a.x);
	int32_t y = (b.y-a.y)*(b.y-a.y);
	int32_t z = (b.z-a.z)*(b.z-a.z);
	return abs(std::sqrt(double(x+y+z)));
}

std::string GetInt3(Int3 position) {
	return "( " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")";
}
std::string GetVec3(Vec3 position) {
	return "( " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")";
}

Int3 LocalToGlobalPosition(Int3 chunkPos, Int3 blockPos) {
	return Int3 {
        chunkPos.x*CHUNK_WIDTH_X + blockPos.x,
        blockPos.y,
        chunkPos.z*CHUNK_WIDTH_Z + blockPos.z
    };
}

Int3 BlockToChunkPosition(Int3 position) {
	position.x = position.x >> 4;
	position.y = 0;
	position.z = position.z >> 4;
	return position;
}

Int3 BlockToChunkPosition(Vec3 position) {
	Int3 intPos = Vec3ToInt3(position);
	return BlockToChunkPosition(intPos);
}

void BlockToFace(int32_t& x, int8_t& y, int32_t& z, int8_t& direction) {
	switch(direction) {
		case 0:
			y--;
			break;
		case 1:
			y++;
			break;
		case 2:
			z--;
			break;
		case 3:
			z++;
			break;
		case 4:
			x--;
			break;
		case 5:
			x++;
			break;
		default:
			break;		
	}
}

int8_t EntryToByte(char* message, int32_t& offset) {
	int8_t result = message[offset];
	offset++;
	return result;
}

int16_t EntryToShort(char* message, int32_t& offset) {
	int16_t result = message[offset] << 8 | message[offset+1];
	offset+=2;
	return result;
}

int32_t EntryToInteger(char* message, int32_t& offset) {
	int32_t result = (message[offset] << 24 | message[offset+1] << 16 | message[offset+2] << 8 | message[offset+3]);
	offset+=4;
	return result;
}

int64_t EntryToLong(char* message, int32_t& offset) {
	int64_t result = (
		(int64_t)message[offset  ] << 56 |
		(int64_t)message[offset+1] << 48 |
		(int64_t)message[offset+2] << 40 |
		(int64_t)message[offset+3] << 32 |
		(int64_t)message[offset+4] << 24 |
		(int64_t)message[offset+5] << 16 |
		(int64_t)message[offset+6] <<  8 |
		(int64_t)message[offset+7]
	);
	offset+=8;
	return result;
}

float EntryToFloat(char* message, int32_t& offset) {
    uint32_t intBits = (static_cast<uint8_t>(message[offset]) << 24) |
                       (static_cast<uint8_t>(message[offset + 1]) << 16) |
                       (static_cast<uint8_t>(message[offset + 2]) << 8) |
                       (static_cast<uint8_t>(message[offset + 3]));
    float result;
    std::memcpy(&result, &intBits, sizeof(result)); // Copy bits into a float
    offset += 4;
    return result;
}

double EntryToDouble(char* message, int32_t& offset) {
    uint64_t intBits = (static_cast<uint64_t>(static_cast<uint8_t>(message[offset])) << 56) |
                       (static_cast<uint64_t>(static_cast<uint8_t>(message[offset + 1])) << 48) |
                       (static_cast<uint64_t>(static_cast<uint8_t>(message[offset + 2])) << 40) |
                       (static_cast<uint64_t>(static_cast<uint8_t>(message[offset + 3])) << 32) |
                       (static_cast<uint64_t>(static_cast<uint8_t>(message[offset + 4])) << 24) |
                       (static_cast<uint64_t>(static_cast<uint8_t>(message[offset + 5])) << 16) |
                       (static_cast<uint64_t>(static_cast<uint8_t>(message[offset + 6])) << 8) |
                       (static_cast<uint64_t>(static_cast<uint8_t>(message[offset + 7])));
    double result;
    std::memcpy(&result, &intBits, sizeof(result)); // Copy bits into a double
    offset += 8; // Correctly increment offset by 8 for double
    return result;
}

// Turns out this is just UTF-8, not Strings with an 8-Bit Length
std::string EntryToString8(char* message, int32_t& offset) {
	std::string string8 = "";
	int16_t stringLength = EntryToShort(message, offset);
	offset++;
	for (int16_t i = 0; i < stringLength; i++) {
		string8 += message[i + offset];
	}
	offset+=stringLength;
	return string8;
}

// Turns out this is just UTF-16, not Strings with a 16-Bit Length
std::string EntryToString16(char* message, int32_t& offset) {
	std::string string16 = "";
	int16_t stringLength = EntryToShort(message, offset);
	for (int16_t i = 0; i < stringLength*2; i+=2) {
		string16 += message[i+offset] << 8 | message[i+offset+1];
	}
	offset+=stringLength*2;
	return string16;
}

void AppendShortToVector(std::vector<uint8_t> &vector, int16_t value) {
	uint8_t byte1 = (value >> 8 ) & 0xFF;
	uint8_t byte0 = (value 		) & 0xFF;
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void AppendIntegerToVector(std::vector<uint8_t> &vector, int32_t value) {
	uint8_t byte3 = (value >> 24) & 0xFF;
	uint8_t byte2 = (value >> 16) & 0xFF;
	uint8_t byte1 = (value >> 8 ) & 0xFF;
	uint8_t byte0 = (value 		) & 0xFF;
	vector.push_back(byte3);
	vector.push_back(byte2);
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void AppendLongToVector(std::vector<uint8_t> &vector, int64_t value) {
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

void AppendFloatToVector(std::vector<uint8_t> &vector, float value) {
    uint32_t intValue;
    std::memcpy(&intValue, &value, sizeof(float));
	uint8_t byte3 = (intValue >> 24) & 0xFF;
	uint8_t byte2 = (intValue >> 16) & 0xFF;
	uint8_t byte1 = (intValue >> 8 ) & 0xFF;
	uint8_t byte0 = (intValue      ) & 0xFF;
	vector.push_back(byte3);
	vector.push_back(byte2);
	vector.push_back(byte1);
	vector.push_back(byte0);
}

void AppendDoubleToVector(std::vector<uint8_t> &vector, double value) {
    uint64_t intValue;
    std::memcpy(&intValue, &value, sizeof(double));
	uint8_t byte7 = (intValue >> 56) & 0xFF;
	uint8_t byte6 = (intValue >> 48) & 0xFF;
	uint8_t byte5 = (intValue >> 40) & 0xFF;
	uint8_t byte4 = (intValue >> 32) & 0xFF;
	uint8_t byte3 = (intValue >> 24) & 0xFF;
	uint8_t byte2 = (intValue >> 16) & 0xFF;
	uint8_t byte1 = (intValue >> 8 ) & 0xFF;
	uint8_t byte0 = (intValue      ) & 0xFF;
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
	int8_t stringLength = value.size();
	AppendShortToVector(vector, stringLength);
	for (int8_t i = 0; i < stringLength; i++) {
		vector.push_back(value[i]);
	}
}

void AppendString16ToVector(std::vector<uint8_t> &vector, std::string value) {
	// Apparently strings support variable lengths, but in this case I'll always assume they're string16s
	int16_t stringLength = value.size();
	AppendShortToVector(vector, stringLength);
	for (int16_t i = 0; i < stringLength; i++) {
		vector.push_back(0);
		vector.push_back(value[i]);
	}
}

int8_t ConvertFloatToPackedByte(float value) {
	return static_cast<int8_t>((value/360.0)*255.0);
}

Vec3 SubtractVec3(Vec3 previousPosition, Vec3 currentPosition) {
	Vec3 difference = previousPosition;
	difference.x -= currentPosition.x;
	difference.y -= currentPosition.y;
	difference.z -= currentPosition.z;
	return difference;
}

Int3 Vec3ToRelativeInt3(Vec3 previousPosition, Vec3 currentPosition) {
	Vec3 difference = SubtractVec3(previousPosition, currentPosition);
	return Int3 {
		static_cast<int8_t>(difference.x*32.0),
		static_cast<int8_t>(difference.y*32.0),
		static_cast<int8_t>(difference.z*32.0)
	};
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

std::string PacketIdToLabel(Packet packet) {
	uint8_t id = (uint8_t)packet;
	if (id == 255) {
		return "0xFF Disconnect";
	}
	if (id > packetLabels.size()) {
		return std::to_string(id);
	}
	return packetLabels[id];
}

int16_t GetBlockIndex(Int3 position) {
    return (int32_t)((int8_t)position.y + position.z*CHUNK_HEIGHT + (position.x*CHUNK_HEIGHT*CHUNK_WIDTH_Z));
}

std::unique_ptr<char[]> CompressChunk(char* chunk, size_t &compressed_size) {

    // Create a compression context
    struct libdeflate_compressor *compressor = libdeflate_alloc_compressor(9);
    if (!compressor) {
        std::cerr << "Failed to allocate compressor\n";
        return nullptr;
    }

    // Allocate space for compressed data
    size_t max_compressed_size = libdeflate_zlib_compress_bound(compressor, CHUNK_DATA_SIZE);
    auto compressed_data = std::make_unique<char[]>(max_compressed_size);

    // Compress the data
    compressed_size = libdeflate_zlib_compress(compressor, chunk, CHUNK_DATA_SIZE,
                                                      compressed_data.get(), max_compressed_size);

    if (compressed_size == 0) {
        std::cerr << "Compression failed\n";
        libdeflate_free_compressor(compressor);
        return nullptr;
    }
    libdeflate_free_compressor(compressor);

    return compressed_data;
}

std::unique_ptr<char[]> DecompressChunk(const char* compressed_data, size_t compressed_size, size_t& decompressed_size) {
    // Create a decompression context
    struct libdeflate_decompressor* decompressor = libdeflate_alloc_decompressor();
    if (!decompressor) {
        std::cerr << "Failed to allocate decompressor\n";
        return nullptr;
    }

    // Estimate the maximum size of the decompressed data
    decompressed_size = CHUNK_DATA_SIZE; // This is an estimate, adjust as needed.
    auto decompressed_data = std::make_unique<char[]>(decompressed_size);

    // Decompress the data
    int result = libdeflate_zlib_decompress(decompressor, compressed_data, compressed_size,
                                             decompressed_data.get(), decompressed_size, &decompressed_size);

    // Check if decompression succeeded
    if (result != LIBDEFLATE_SUCCESS) {
        std::cerr << "Decompression failed with error code " << result << std::endl;
        libdeflate_free_decompressor(decompressor);
        return nullptr;
    }

    // Now decompressed_size is correctly set
    libdeflate_free_decompressor(decompressor);
    return decompressed_data;
}


int64_t GetChunkHash(int32_t x, int32_t z) {
    return ((int64_t)x << 32) | (z & 0xFFFFFFFF);
}

Int3 DecodeChunkHash(int64_t hash) {
    return Int3 {
        (int32_t)(hash >> 32),
        0,
        (int32_t)(hash & 0xFFFFFFFF)
    };
}

Int3 Int3ToEntityInt3(Int3 pos) {
	return Int3 {
		pos.x << 5 | 16,
		pos.y << 5 | 16,
		pos.z << 5 | 16
	};
}

Int3 Vec3ToEntityInt3(Vec3 pos) {
	return Int3 {
		int32_t(pos.x*32),
		int32_t(pos.y*32),
		int32_t(pos.z*32)
	};
}

Vec3 EntityInt3ToVec3(Int3 pos) {
	return Vec3 {
		double(pos.x)/32,
		double(pos.y)/32,
		double(pos.z)/32
	};
}