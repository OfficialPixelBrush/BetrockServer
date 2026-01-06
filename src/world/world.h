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

/**
 * @brief Responsible for saving, loading and holding onto chunks
 * 
 */
class World {
  private:
	std::unordered_map<std::string, std::shared_ptr<RegionFile>> openRegions;
	std::mutex regionMutex;

	std::unordered_map<int64_t, std::shared_ptr<Chunk>> chunks;
	std::deque<LightUpdate> lightingToUpdate;
	std::filesystem::path dirPath;
	void RemoveChunk(Int2 position);
	std::random_device dev;
	std::mt19937 rng;
	bool RandomTick(Int3 &pos);
	mutable std::shared_mutex lightUpdateMutex;
	mutable std::shared_mutex chunkMutex;
	std::atomic<int32_t> lightingUpdatesCounter{0};
	std::atomic<int32_t> lightingUpdatesScheduled{0};
	bool MergeBox(Int3 &posA, Int3 &posB, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6);

  public:
	int64_t seed;
	World(const std::string &extra = "");

	// Block-related
	void PlaceBlock(Int3 position, BlockType type = BLOCK_AIR, int8_t meta = 0);
	void PlaceBlockUpdate(Int3 position, BlockType type = BLOCK_AIR, int8_t meta = 0, bool sendUpdate = true);
	void PlaceSponge(Int3 position);
	bool BlockExists(Int3 position);

	BlockType GetBlockType(Int3 pos);
	void SetBlockType(BlockType blockType, Int3 pos);
	int8_t GetBlockMeta(Int3 pos);
	void SetBlockMeta(int8_t blockMeta, Int3 pos);

	void SetBlockTypeAndMeta(BlockType blockType, int8_t blockMeta, Int3 pos);

	int8_t GetBlockLight(Int3 pos);
	void SetBlockLight(int8_t value, Int3 pos);
	int8_t GetSkyLight(Int3 pos);
	void SetSkyLight(int8_t value, Int3 pos);

	void UpdateBlock(Int3 position);
	bool InteractWithBlock(Int3 pos);

	// Light-related
	void SpreadLight(bool skyLight, Int3 pos, int32_t limit);
	void ScheduleLightingUpdate(bool skyLight, Int3 pos1, Int3 pos2, bool checkDuplicates = true);
	bool UpdatingLighting();
	void ProcessSingleLightUpdate(const LightUpdate &current);
	void SetLight(bool skyLight, Int3 pos, int8_t newLight);
	int8_t GetLight(bool skyLight, Int3 pos);
	int8_t GetTotalLight(Int3 pos);
	bool CanBlockSeeTheSky(Int3 pos);

	// Chunk-related
	void GetChunkData(uint8_t *chunkData, Int2 position);
	std::vector<SignTile *> GetChunkSigns(Int2 position);
	void TickChunks();
	std::shared_ptr<Chunk> GetChunk(Int2 position);
	bool IsChunkPopulated(Int2 position);
	bool IsChunkGenerated(Int2 position);
	std::shared_ptr<Chunk> AddChunk(Int2 position, std::shared_ptr<Chunk> c);
	void FreeUnseenChunks();
	void SaveChunk(Int2 position, std::shared_ptr<Chunk> chunk);
	std::shared_ptr<Chunk> LoadMcRegionChunk(Int2 position);
	std::shared_ptr<Chunk> LoadOldV2Chunk(Int2 position);
	std::shared_ptr<Chunk> LoadOldChunk(Int2 position);
	bool ChunkFileExists(Int2 position, std::string extension = std::string(CHUNK_FILE_EXTENSION));
	bool ChunkExists(Int2 position);

	// Misc
	void Save();
	int32_t GetNumberOfChunks();
	int32_t GetNumberOfPopulatedChunks();
	int32_t GetNumberOfModifiedChunks();
	int8_t GetHeightValue(Int2 position);
	int8_t GetFirstUncoveredBlock(Int3 &position);
	void AddTileEntity(std::unique_ptr<TileEntity> &&te);
	TileEntity *GetTileEntity(Int3 position);
	int32_t GetHighestSolidOrLiquidBlock(Int2 position);
};