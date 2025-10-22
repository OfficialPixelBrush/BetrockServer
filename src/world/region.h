#include <cstdint>
#include <filesystem>

#define SECTOR_SIZE 4096
#define REGION_CHUNKS_X 32
#define REGION_CHUNKS_Z 32

class RegionFile {
    private:
        std::filesystem::path filePath;
        int32_t offsets[1024];
        int32_t timestamps[1024];
    public:
        RegionFile(std::filesystem::path filePath);
        void Write();
        int32_t GetChunkOffset(int32_t cX, int32_t cZ);
};

class RegionHandler {
    public: 
        RegionHandler();

};