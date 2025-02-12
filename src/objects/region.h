#pragma once 
#include <cstdint>
#include <unordered_map>
#include <fstream>
#include <filesystem>

#include "helper.h"

#define SECTOR_SIZE 4096

class Region {
    private:
        int32_t offset = 0;
        std::unordered_map<int64_t,std::shared_ptr<Chunk>> chunks;
    public:
        void AddChunk(const int64_t& hash, const Chunk& chunk);
        void Save(std::filesystem::path dirPath);
};