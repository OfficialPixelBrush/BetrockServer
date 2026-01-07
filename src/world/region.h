#pragma once
#include <cstdint>
#include <filesystem>
#include <fstream>

#include "helper.h"
#include "pixnbt.h"

#define SECTOR_SIZE 4096
#define REGION_CHUNKS_X 32
#define REGION_CHUNKS_Z 32
#define MCREGION_CHUNKS REGION_CHUNKS_X * REGION_CHUNKS_Z // 1024

class RegionFile {
  public:
	std::fstream regionStream;
	std::filesystem::path filePath;
	uint32_t offsets[MCREGION_CHUNKS];
	uint32_t timestamps[MCREGION_CHUNKS];
	std::vector<bool> freeSectors;
	size_t sizeDelta = 0;
	std::mutex streamMutex;

	RegionFile(std::filesystem::path filePath);
	void Write();
	std::fstream GetChunkDataStream(Int2 position);
	std::shared_ptr<CompoundNbtTag> GetChunkNbt(Int2 position);

  private:
	bool IsOutOfBounds(Int2 position);
	int32_t SetChunkOffset(Int2 position);
	int32_t GetChunkOffset(Int2 position);
};

/*
class RegionManager {
	std::unordered_map<std::string, std::shared_ptr<RegionFile>> openRegions;
	public:
		RegionManager();

};
*/