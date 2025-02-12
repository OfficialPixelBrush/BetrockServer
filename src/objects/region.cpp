#include "region.h"

void Region::AddChunk(const int64_t& hash, const Chunk& chunk) {
    chunks[hash] = std::make_shared<Chunk>(chunk);
}

void Region::Save(std::filesystem::path dirPath) {
    for (const auto& pair : chunks) {
        const int64_t& hash = pair.first;
        const std::shared_ptr<Chunk>& chunk = pair.second;
        
        offset = offset & 0x0FFF; // Limit offset to 3-Bytes

        Int3 pos = DecodeChunkHash(hash);
        Int3 regionPos = ChunkToRegionPosition(pos);

        std::filesystem::path filePath = dirPath / ("r." + std::to_string(regionPos.x) + "." + std::to_string(regionPos.z) + ".mcr");

        // Open file for writing
        std::ofstream chunkFile (filePath);
        if (!chunkFile) {
            std::cerr << "Failed to save region at " << regionPos.x << ", " << regionPos.z << '\n';
            continue;
        }

        // Acquire existing chunk data
        // TODO: Get Chunk Data
        std::unique_ptr<char[]> chunkData = nullptr; //GetChunkData(pos);
        // Add it to the list of chunks to be compressed and sent
        if (!chunkData) {
            std::cout << "Failed to get Chunk " << pos << std::endl;
            continue;
        }

        size_t compressedSize = 0;
        auto chunkBinary = CompressChunk(chunkData.get(), compressedSize);
        
        if (!chunkBinary || compressedSize == 0) {		
            std::cout << "Failed to compress Chunk " << pos << std::endl;
            continue;
        }

        // Figure out where the chunk pointer goes
        size_t chunkLocationPointer = 4 * ((pos.x & 31) + (pos.z & 31) * 32);

        chunkFile.seekp(chunkLocationPointer, std::ios::beg);
        chunkFile << offset;
        chunkFile.seekp(SECTOR_SIZE*2 + offset, std::ios::beg);
        int8_t sectorCount = (int8_t)std::ceil((float)compressedSize/SECTOR_SIZE);
        offset += sectorCount;

        chunkFile.write(chunkBinary.get(), compressedSize);
        chunkFile.close();
    }
}