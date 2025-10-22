#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>

#include "helper.h"
#include "nbt.h"

#define SECTOR_SIZE 4096
#define REGION_CHUNKS_X 32
#define REGION_CHUNKS_Z 32
#define MCREGION_CHUNKS REGION_CHUNKS_X*REGION_CHUNKS_Z // 1024

class RegionFile {
    public:
        std::fstream regionStream;
        std::filesystem::path filePath;
        uint32_t offsets[MCREGION_CHUNKS];
        uint32_t timestamps[MCREGION_CHUNKS];
        std::vector<bool> freeSectors;
        size_t sizeDelta = 0;

        RegionFile(std::filesystem::path filePath);
        void Write();
        std::fstream GetChunkDataStream(int32_t cX, int32_t cZ);
        std::shared_ptr<CompoundTag> GetChunkNbt(int32_t cX, int32_t cZ);
    private:
        bool IsOutOfBounds(int32_t cX, int32_t cZ);
        int32_t SetChunkOffset(int32_t cX, int32_t cZ);
        int32_t GetChunkOffset(int32_t cX, int32_t cZ);
};

class RegionHandler {
    public: 
        RegionHandler();

};