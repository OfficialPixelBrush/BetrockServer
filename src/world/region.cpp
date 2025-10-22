#include "region.h"

RegionFile::RegionFile(std::filesystem::path filePath) {
    this->filePath = filePath;

    
}

int32_t RegionFile::GetChunkOffset(int32_t cX, int32_t cZ) {
    return this->offsets[cX + cZ * REGION_CHUNKS_X];
}