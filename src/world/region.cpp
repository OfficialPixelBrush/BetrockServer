#include "region.h"

RegionFile::RegionFile(std::filesystem::path pFilePath) {
	this->filePath = pFilePath;
	// regionStream.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
	//  Read-only for testing
	regionStream.open(filePath, std::ios::in | std::ios::binary);

	// If the file is less than one sector, its clearly messed up
	// std::cout << "Filesize: " << std::to_string(GetFileSize(regionStream)) << "\n";
	if (GetFileSize(regionStream) < SECTOR_SIZE) {
		std::cout << "File less than 1 Sector" << "\n";
		// Fill with zeroes
		// Writes 1024 * 2 ints (8192 Bytes)
		int32_t zero = 0;
		for (size_t i = 0; i < MCREGION_CHUNKS * 2; i++) {
			regionStream.write(reinterpret_cast<char *>(&zero), sizeof(zero));
		}
		sizeDelta += MCREGION_CHUNKS * 8;
	}
	// std::cout << "Ensured that file is at least 8K" << "\n";

	// Pads the file up to the next sector boundary
	if ((GetFileSize(regionStream) & (SECTOR_SIZE - 1)) != 0) {
		for (size_t i = 0; i < (GetFileSize(regionStream) & size_t(SECTOR_SIZE - 1)); i++) {
			uint8_t zero = 0;
			regionStream.write(reinterpret_cast<char *>(&zero), sizeof(zero));
		}
		// std::cout << "Padded to next boundary" << "\n";
	}

	size_t numberOfSectors = (GetFileSize(regionStream) / size_t(SECTOR_SIZE));
	// freeSectors.reserve(numberOfSectors);

	for (size_t i = 0; i < numberOfSectors; i++) {
		freeSectors.push_back(true);
	}
	// std::cout << std::to_string(numberOfSectors) << "\n";
	//  Mark the offset and timestamp sectors to used
	freeSectors[0] = false;
	freeSectors[1] = false;

	// Reset to start of file
	regionStream.seekg(0);

	// Read offsets (Big Endian)
	for (size_t i = 0; i < MCREGION_CHUNKS; i++) {
		uint32_t offset = 0;
		regionStream.read(reinterpret_cast<char *>(&offset), sizeof(offset));
		offset = Swap32(offset);
		// std::cout << std::hex << offset << ",";
		offsets[i] = offset;
		if ((offset != 0 && (offset >> 8) + (offset & 0xFF)) <= freeSectors.size()) {
			for (uint32_t sectorSet = 0; sectorSet < (offset & 0xFF); sectorSet++) {
				freeSectors[(offset >> 8) + sectorSet] = false;
			}
		}
	}
	// std::cout << "Read Offsets" << "\n";

	// Read Timestamps
	for (int32_t i = 0; i < MCREGION_CHUNKS; i++) {
		uint32_t timestamp = 0;
		regionStream.read(reinterpret_cast<char *>(&timestamp), sizeof(timestamp));
		timestamps[i] = Swap32(timestamp);
	}
	// std::cout << "Read Timestamps" << "\n";
}

int32_t RegionFile::GetChunkOffset(Int2 position) { return this->offsets[position.x + position.y * REGION_CHUNKS_X]; }

std::shared_ptr<CompoundTag> RegionFile::GetChunkNbt(Int2 position) {
	streamMutex.lock();
	std::lock_guard lock(streamMutex);
	if (!regionStream.good()) {
		std::cout << "Stream read failed, possibly corrupt region\n";
		return nullptr;
	}

	// Put into bounds
	position.x = (position.x & 31);
	position.y = (position.y & 31);
	// Check if chunk is within bounds
	if (IsOutOfBounds(position)) {
		std::cout << "READ " << position << " out of bounds" << "\n";
		return nullptr;
	}

	// Get the chunk offset
	int32_t offset = GetChunkOffset(position);
	if (!offset) {
		std::cout << "READ " << position << " invalid offset" << "\n";
		return nullptr;
	}

	int32_t upperOffset = offset >> 8;
	int32_t lowerOffset = offset & 0xFF;
	if (upperOffset + lowerOffset > int32_t(freeSectors.size())) {
		std::cout << "READ " << position << " invalid sector" << "\n";
		return nullptr;
	}

	// Move the the relevant sector
	regionStream.seekg((upperOffset * SECTOR_SIZE));
	uint32_t dataLength = 0;
	regionStream.read(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
	dataLength = Swap32(dataLength);
	if (dataLength > uint32_t(SECTOR_SIZE * lowerOffset)) {
		std::cout << "READ " << position << " invalid length "
				  << std::to_string(dataLength) << " > 4096 * " << std::to_string(lowerOffset) << "\n";
		return nullptr;
	}

	// Read the compression format indicator byte
	uint8_t compressionFormat = 0;
	regionStream.read(reinterpret_cast<char *>(&compressionFormat), sizeof(compressionFormat));

	// Read the binary data into a buffer
	std::vector<uint8_t> byteBuffer(dataLength - 1);
	regionStream.read(reinterpret_cast<char *>(byteBuffer.data()), byteBuffer.size());

	std::stringstream dataStream(std::string(byteBuffer.begin(), byteBuffer.end()));
	switch (compressionFormat) {
	case 1:
		// Gzip compressed
		return std::dynamic_pointer_cast<CompoundTag>(NbtRead(dataStream, NBT_GZIP, -1, CHUNK_DATA_SIZE * 20));
	case 2:
		// ZLib compressed
		return std::dynamic_pointer_cast<CompoundTag>(NbtRead(dataStream, NBT_ZLIB, -1, CHUNK_DATA_SIZE * 20));
	}

	std::cout << "READ " << position << " unknown version "
			  << std::to_string(compressionFormat) << "\n";
	return nullptr;
}

bool RegionFile::IsOutOfBounds(Int2 position) {
	return position.x < 0 || position.x >= REGION_CHUNKS_X || position.y < 0 || position.y >= REGION_CHUNKS_Z;
}