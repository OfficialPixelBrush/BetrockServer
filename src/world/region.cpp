#include "region.h"

RegionFile::RegionFile(std::filesystem::path filePath) {
    this->filePath = filePath;
    //regionStream.open(filePath, std::ios::in | std::ios::out | std::ios::binary);
    // Read-only for testing
    regionStream.open(filePath, std::ios::in | std::ios::binary);

    // If the file is less than one sector, its clearly messed up
    //std::cout << "Filesize: " << std::to_string(GetFileSize(regionStream)) << std::endl;
    if (GetFileSize(regionStream) < SECTOR_SIZE) {
        std::cout << "File less than 1 Sector" << std::endl;
        // Fill with zeroes
        // Writes 1024 * 2 ints (8192 Bytes)
        int32_t zero = 0;
        for (size_t i = 0; i < MCREGION_CHUNKS*2; i++) {
            regionStream.write(reinterpret_cast<char*>(&zero), sizeof(zero));
        }
        sizeDelta += MCREGION_CHUNKS*8;
    }
    //std::cout << "Ensured that file is at least 8K" << std::endl;

    // Pads the file up to the next sector boundary
    if ((GetFileSize(regionStream) & (SECTOR_SIZE - 1)) != 0) {
        for (size_t i = 0; i < (GetFileSize(regionStream) & size_t(SECTOR_SIZE - 1)); i++) {
            uint8_t zero = 0;
            regionStream.write(reinterpret_cast<char*>(&zero), sizeof(zero));
        }
        //std::cout << "Padded to next boundary" << std::endl;
    }

    size_t numberOfSectors = (GetFileSize(regionStream) / size_t(SECTOR_SIZE));
    //freeSectors.reserve(numberOfSectors);

    for (size_t i = 0; i < numberOfSectors; i++) {
        freeSectors.push_back(true);
    }
    //std::cout << std::to_string(numberOfSectors) << std::endl;
    // Mark the offset and timestamp sectors to used
    freeSectors[0] = false;
    freeSectors[1] = false;

    // Reset to start of file
    regionStream.seekg(0);

    // Read offsets (Big Endian)
    for (size_t i = 0; i < MCREGION_CHUNKS; i++) {
        uint32_t offset = 0;
        regionStream.read(reinterpret_cast<char*>(&offset), sizeof(offset));
        offset = Swap32(offset);
        //std::cout << std::hex << offset << ",";
        offsets[i] = offset;
        if (
            (offset != 0 && (offset >> 8) + (offset & 0xFF))
            <= freeSectors.size()
        ) {
            for (uint sectorSet = 0; sectorSet < (offset & 0xFF); sectorSet++) {
                freeSectors[(offset >> 8) + sectorSet] = false;
            }
        }
    }
    //std::cout << "Read Offsets" << std::endl;

    // Read Timestamps
    for (int i = 0; i < MCREGION_CHUNKS; i++) {
        uint32_t timestamp = 0;
        regionStream.read(reinterpret_cast<char*>(&timestamp), sizeof(timestamp));
        timestamps[i] = Swap32(timestamp);
    }
    //std::cout << "Read Timestamps" << std::endl;
}

int32_t RegionFile::GetChunkOffset(int32_t cX, int32_t cZ) {
    return this->offsets[cX + cZ * REGION_CHUNKS_X];
}

std::shared_ptr<CompoundTag> RegionFile::GetChunkNbt(int32_t cX, int32_t cZ) {
    // Check if chunk is within bounds
    if (IsOutOfBounds(cX, cZ)) return nullptr;

    // Get the chunk offset
    int32_t offset = GetChunkOffset(cX, cZ);
    if (!offset) return nullptr;

    int32_t upperOffset = offset >> 8;
    int32_t lowerOffset = offset & 0xFF;
    if(upperOffset + lowerOffset > int32_t(freeSectors.size())) {
        std::cout << "READ " << std::to_string(cX) << ", " << std::to_string(cZ) << " invalid sector" << std::endl;
        return nullptr;
    }

    // Move the the relevant sector
    regionStream.seekg((upperOffset * SECTOR_SIZE));
    uint32_t dataLength = 0;
    regionStream.read(reinterpret_cast<char*>(&dataLength), sizeof(dataLength));
    dataLength = Swap32(dataLength);
    if(dataLength > uint32_t(SECTOR_SIZE * lowerOffset)) {
        std::cout << "READ " << std::to_string(cX) << ", " << std::to_string(cZ) << " invalid length " << std::to_string(dataLength) << " > 4096 * " << std::to_string(lowerOffset) << std::endl;
        return nullptr;
    }

    // Read the compression format indicator byte
    uint8_t compressionFormat = 0;
    regionStream.read(reinterpret_cast<char*>(&compressionFormat), sizeof(compressionFormat));

    // Read the binary data into a buffer
    std::vector<uint8_t> byteBuffer(dataLength - 1);
    regionStream.read(reinterpret_cast<char*>(byteBuffer.data()), byteBuffer.size());

    std::stringstream dataStream(std::string(byteBuffer.begin(), byteBuffer.end()));
    switch(compressionFormat) {
        case 1:
            // Gzip compressed
            return std::dynamic_pointer_cast<CompoundTag>(NbtRead(dataStream, NBT_GZIP, -1, CHUNK_DATA_SIZE*10));
        case 2:
            // ZLib compressed
            return std::dynamic_pointer_cast<CompoundTag>(NbtRead(dataStream, NBT_ZLIB, -1, CHUNK_DATA_SIZE*10));
    }

    std::cout << "READ " << std::to_string(cX) << ", " << std::to_string(cZ) << " unknown version " << std::to_string(compressionFormat) << std::endl;
    return nullptr;
}

bool RegionFile::IsOutOfBounds(int cX, int cZ) {
    return cX < 0 || cX >= REGION_CHUNKS_X || cZ < 0 || cZ >= REGION_CHUNKS_Z;
}