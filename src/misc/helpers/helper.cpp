#include "helper.h"

// Check if the passed value is between a and b
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

// Get the Euclidian distance between two Vec3s
double GetEuclidianDistance(Vec3 a, Vec3 b) {
	double x = (b.x-a.x)*(b.x-a.x);
	double y = (b.y-a.y)*(b.y-a.y);
	double z = (b.z-a.z)*(b.z-a.z);
	return abs(std::sqrt(x+y+z));
}

// Get the Euclidian distance between two Int3s
double GetEuclidianDistance(Int3 a, Int3 b) {
	int32_t x = (b.x-a.x)*(b.x-a.x);
	int32_t y = (b.y-a.y)*(b.y-a.y);
	int32_t z = (b.z-a.z)*(b.z-a.z);
	return abs(std::sqrt(double(x+y+z)));
}

// Get the Taxicab distance between two Vec3s
double GetTaxicabDistance(Vec3 a, Vec3 b) {
	double x = abs(a.x-b.x);
	double y = abs(a.y-b.y);
	double z = abs(a.z-b.z);
	return x+y+z;
}

// Get the Taxicab distance between two Int3s
double GetTaxicabDistance(Int3 a, Int3 b) {
	int32_t x = abs(a.x-b.x);
	int32_t y = abs(a.y-b.y);
	int32_t z = abs(a.z-b.z);
	return double(x+y+z);
}

// Get the Chebyshev distance between two Vec3s
double GetChebyshevDistance(Vec3 a, Vec3 b) {
	double x = abs(a.x-b.x);
	double y = abs(a.y-b.y);
	double z = abs(a.z-b.z);
	return std::max(std::max(x,y),z);
}

// Get the Chebyshev distance between two Int3s
double GetChebyshevDistance(Int3 a, Int3 b) {
	int32_t x = abs(a.x-b.x);
	int32_t y = abs(a.y-b.y);
	int32_t z = abs(a.z-b.z);
	return std::max(std::max(x,y),z);
}

// Turn an Int3 into a string
std::string GetInt3(Int3 position) {
	return "( " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")";
}
// Turn an Vec3 into a string
std::string GetVec3(Vec3 position) {
	return "( " + std::to_string(position.x) + ", " + std::to_string(position.y) + ", " + std::to_string(position.z) + ")";
}

// Determine the global position based on where within the passed chunk position the block position is
Int3 LocalToGlobalPosition(Int3 chunkPos, Int3 blockPos) {
	return Int3 {
        chunkPos.x*CHUNK_WIDTH_X + blockPos.x,
        blockPos.y,
        chunkPos.z*CHUNK_WIDTH_Z + blockPos.z
    };
}

// Get the Chunk Position of an Int3 coordinate
Int3 BlockToChunkPosition(Int3 position) {
	position.x = position.x >> 4;
	position.y = 0;
	position.z = position.z >> 4;
	return position;
}

// Get the Chunk Position of a Vec3 coordinate
Int3 BlockToChunkPosition(Vec3 position) {
	Int3 intPos = Vec3ToInt3(position);
	return BlockToChunkPosition(intPos);
}

// Turn a float value into a byte, mapping the range 0-255 to 0°-360°
int8_t ConvertFloatToPackedByte(float value) {
	return static_cast<int8_t>((value/360.0)*255.0);
}

constexpr std::array<const char*, 256> packetLabels = [] {
    std::array<const char*, 256> labels{};
    labels.fill("Unknown Packet"); // Default value for unhandled IDs

    labels[0x00] = "0x00 KeepAlive";
    labels[0x01] = "0x01 LoginRequest";
    labels[0x02] = "0x02 Handshake";
    labels[0x03] = "0x03 ChatMessage";
    labels[0x04] = "0x04 TimeUpdate";
    labels[0x05] = "0x05 EntityEquipment";
    labels[0x06] = "0x06 SpawnPosition";
    labels[0x07] = "0x07 UseEntity";
    labels[0x08] = "0x08 UpdateHealth";
    labels[0x09] = "0x09 Respawn";
    labels[0x0A] = "0x0A Player";
    labels[0x0B] = "0x0B PlayerPosition";
    labels[0x0C] = "0x0C PlayerLook";
    labels[0x0D] = "0x0D PlayerPositionLook";
    labels[0x0E] = "0x0E PlayerDigging";
    labels[0x0F] = "0x0F PlayerBlockPlacement";
    labels[0x10] = "0x10 HoldingChange";
    labels[0x11] = "0x11 UseBed";
    labels[0x12] = "0x12 Animation";
    labels[0x13] = "0x13 EntityAction???";
    labels[0x14] = "0x14 NamedEntitySpawn";
    labels[0x15] = "0x15 PickupSpawn";
    labels[0x16] = "0x16 CollectItem";
    labels[0x17] = "0x17 AddObjectVehicle";
    labels[0x18] = "0x18 MobSpawn";
    labels[0x19] = "0x19 Painting";
    labels[0x1C] = "0x1C EntityVelocity?";
    labels[0x1D] = "0x1D DestroyEntity";
    labels[0x1E] = "0x1E Entity";
    labels[0x1F] = "0x1F EntityRelativeMove";
    labels[0x20] = "0x20 EntityLook";
    labels[0x21] = "0x21 EntityLookAndRelativeMove";
    labels[0x22] = "0x22 EntityTeleport";
    labels[0x26] = "0x26 EntityStatus?";
    labels[0x27] = "0x27 AttachEntity?";
    labels[0x28] = "0x28 EntityMetadata";
    labels[0x32] = "0x32 PreChunk";
    labels[0x33] = "0x33 Chunk";
    labels[0x34] = "0x34 MultiBlockChange";
    labels[0x35] = "0x35 BlockChange";
    labels[0x36] = "0x36 PlayNoteBlock";
    labels[0x3C] = "0x3C Explosion";
    labels[0x3D] = "0x3D Sound effect";
    labels[0x46] = "0x46 New/Valid State";
    labels[0x47] = "0x47 Thunderbolt";
    labels[0x64] = "0x64 Open window";
    labels[0x65] = "0x65 Close window";
    labels[0x66] = "0x66 Window click";
    labels[0x67] = "0x67 Set slot";
    labels[0x68] = "0x68 Window items";
    labels[0x69] = "0x69 Update progress bar";
    labels[0x6A] = "0x6A Transaction";
    labels[0x82] = "0x82 Update Sign";
    labels[0x83] = "0x83 Map Data";
    labels[0xFF] = "0xFF Disconnect";

    return labels;
}();

// Get the Label of a Packet
std::string PacketIdToLabel(Packet packet) {
    return packetLabels[(uint8_t)packet];
}

// Get the Block position from the Index
Int3 GetBlockPosition(int index) {
    Int3 position;

    position.x = index / (CHUNK_HEIGHT * CHUNK_WIDTH_Z);  // Get x-coordinate
    index %= (CHUNK_HEIGHT * CHUNK_WIDTH_Z);               // Remainder after dividing by width

    position.z = index / CHUNK_HEIGHT;                     // Get z-coordinate
    position.y = index % CHUNK_HEIGHT;                     // Get y-coordinate

    return position;
}


// Compress the passed binary Chunk data
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

// Decompress the passed binary Chunk data
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

// Get the Chunk Hash of a Chunk
int64_t GetChunkHash(int32_t x, int32_t z) {
    return ((int64_t)x << 32) | (z & 0xFFFFFFFF);
}

// Turn the chunk hash into X and Z coordinates
Int3 DecodeChunkHash(int64_t hash) {
    return Int3 {
        (int32_t)(hash >> 32),
        0,
        (int32_t)(hash & 0xFFFFFFFF)
    };
}


// Safely transform a string into an integer
int32_t SafeStringToInt(std::string in) {
	return std::stoi(in);
	try {
	} catch (const std::exception &e) {
		Betrock::Logger::Instance().Warning(e.what());
		return 0;
	}
}

// Safely transform a string into a long
int64_t SafeStringToLong(std::string in) {
	try {
		return std::stol(in);
	} catch (const std::exception &e) {
		Betrock::Logger::Instance().Warning(e.what());
		return 0;
	}
}

// Get the current time as a string
std::string GetRealTime() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
	return ss.str();
}

// Get the current time as a string without any spaces
std::string GetRealTimeFileFormat() {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);

	std::stringstream ss;
	ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d-%H-%M-%S");
	return ss.str();
}

// Create a hex-dump string of the passed uint8_t array
std::string Uint8ArrayToHexDump(const uint8_t* array, size_t size) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (size_t i = 0; i < size; i += 16) {
        // Print offset
        oss << std::setw(4) << i << "  ";

        // Print hex values
        for (size_t j = 0; j < 16; ++j) {
            if (i + j < size) {
                oss << std::setw(2) << static_cast<int>(array[i + j]) << " ";
            } else {
                oss << "   "; // Padding for alignment
            }
        }

        oss << " ";

        // Print ASCII representation
        for (size_t j = 0; j < 16 && i + j < size; ++j) {
            char c = array[i + j];
            oss << (std::isprint(c) ? c : '.');
        }

        oss << std::endl;
    }

    return oss.str();
}

void LimitBlockCoordinates(Int3 &position) {
    position.y = std::max(std::min(position.y,127),0);
}