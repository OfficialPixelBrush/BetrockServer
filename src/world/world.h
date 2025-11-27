#pragma once
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "blocks.h"
#include "chunk.h"
#include "config.h"
#include "generator.h"
#include "helper.h"
#include "region.h"
#include "tileEntity.h"

class Chunk;
class RegionFile;

struct LightUpdate {
	bool skyLight;
	Int3 posA, posB;
	LightUpdate() : skyLight(false), posA(Int3{0, 0, 0}), posB(Int3{0, 0, 0}) {}
	LightUpdate(bool pSkyLight, Int3 pPosA, Int3 pPosB) : skyLight(pSkyLight), posA(pPosA), posB(pPosB) {}
};
typedef struct LightUpdate LightUpdate;

class World {
  private:
	std::unordered_map<std::string, std::shared_ptr<RegionFile>> openRegions;
	std::mutex regionMutex;

	std::unordered_map<long, std::shared_ptr<Chunk>> chunks;
	std::deque<LightUpdate> lightingToUpdate;
	std::filesystem::path dirPath;
	void RemoveChunk(int32_t x, int32_t z);
	std::random_device dev;
	std::mt19937 rng;
	bool RandomTick(Int3 &pos);
	mutable std::shared_mutex lightUpdateMutex;
	mutable std::shared_mutex chunkMutex;
	std::atomic<int> lightingUpdatesCounter{0};
	std::atomic<int> lightingUpdatesScheduled{0};
	bool MergeBox(Int3 &posA, Int3 &posB, int a1, int a2, int a3, int a4, int a5, int a6);

  public:
	int64_t seed;
	World(const std::string &extra = "");

	// Block-related
	void PlaceBlock(Int3 position, int8_t type = BLOCK_AIR, int8_t meta = 0);
	void PlaceBlockUpdate(Int3 position, int8_t type = BLOCK_AIR, int8_t meta = 0, bool sendUpdate = true);
	void PlaceSponge(Int3 position);
	bool BlockExists(Int3 position);

	int8_t GetBlockType(Int3 pos);
	void SetBlockType(int8_t blockType, Int3 pos);
	int8_t GetBlockMeta(Int3 pos);
	void SetBlockMeta(int8_t blockMeta, Int3 pos);

	void SetBlockTypeAndMeta(int8_t blockType, int8_t blockMeta, Int3 pos);

	int8_t GetBlockLight(Int3 pos);
	void SetBlockLight(int8_t value, Int3 pos);
	int8_t GetSkyLight(Int3 pos);
	void SetSkyLight(int8_t value, Int3 pos);

	void UpdateBlock(Int3 position);
	bool InteractWithBlock(Int3 pos);

	// Light-related
	void SpreadLight(bool skyLight, Int3 pos, int limit);
	void ScheduleLightingUpdate(bool skyLight, Int3 pos1, Int3 pos2, bool checkDuplicates = true);
	bool UpdatingLighting();
	void ProcessSingleLightUpdate(const LightUpdate &current);
	void SetLight(bool skyLight, Int3 pos, int8_t newLight);
	int8_t GetLight(bool skyLight, Int3 pos);
	int8_t GetTotalLight(Int3 pos);
	bool CanBlockSeeTheSky(Int3 pos);

	// Chunk-related
	void GetChunkData(uint8_t *chunkData, Int3 position);
	std::vector<SignTile *> GetChunkSigns(Int3 position);
	void TickChunks();
	std::shared_ptr<Chunk> GetChunk(int32_t x, int32_t z);
	bool IsChunkPopulated(int32_t x, int32_t z);
	bool IsChunkGenerated(int32_t x, int32_t z);
	std::shared_ptr<Chunk> AddChunk(int32_t x, int32_t z, std::shared_ptr<Chunk> c);
	void FreeUnseenChunks();
	void SaveChunk(int32_t x, int32_t z, std::shared_ptr<Chunk> chunk);
	std::shared_ptr<Chunk> LoadMcRegionChunk(int32_t cX, int32_t cZ);
	std::shared_ptr<Chunk> LoadOldV2Chunk(int32_t x, int32_t z);
	std::shared_ptr<Chunk> LoadOldChunk(int32_t x, int32_t z);
	bool ChunkFileExists(int32_t x, int32_t z, std::string extension = std::string(CHUNK_FILE_EXTENSION));
	bool ChunkExists(int32_t x, int32_t z);

	// Misc
	void Save();
	int GetNumberOfChunks();
	int GetNumberOfPopulatedChunks();
	int GetNumberOfModifiedChunks();
	int8_t GetHeightValue(int32_t x, int32_t z);
	int8_t GetFirstUncoveredBlock(Int3 &position);
	void AddTileEntity(std::unique_ptr<TileEntity> &&te);
	TileEntity *GetTileEntity(Int3 pos);
	int GetHighestSolidOrLiquidBlock(int32_t x, int32_t z);
};