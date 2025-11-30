#pragma once
#include "blocks.h"
#include "datatypes.h"
#include "tileEntity.h"
#include "world.h"
#include <cstdint>

class World;

enum ChunkState : int8_t {
	Invalid,
	Generating,
	Generated,
	Populating,
	Populated
};

class Chunk {
  private:
	int8_t heightMap[256];
	uint8_t lowestBlockHeight = CHUNK_HEIGHT - 1;
	World *world;
	int32_t xPos, zPos;
	std::vector<std::unique_ptr<TileEntity>> tileEntities;

	void RelightBlock(int var1, int var2, int var3);
	void UpdateSkylight_do(int x, int z);
	void CheckSkylightNeighborHeight(int x, int z, int height);
	// Block* GetBlock(Int3 pos);
	// Block* GetBlock(int32_t x, int8_t y, int32_t z);
	int8_t blockTypeArray[(CHUNK_WIDTH_X * CHUNK_WIDTH_Z * CHUNK_HEIGHT)];
	int8_t blockMetaArray[(CHUNK_WIDTH_X * CHUNK_WIDTH_Z * CHUNK_HEIGHT) / 2];
	int8_t blockLightArray[(CHUNK_WIDTH_X * CHUNK_WIDTH_Z * CHUNK_HEIGHT)];

  public:
	int8_t state = ChunkState::Invalid;
	bool modified = false;
	std::shared_ptr<CompoundTag> GetAsNbt();
	void ReadFromNbt(std::shared_ptr<CompoundTag> readRoot);

	Chunk(World *pWorld, Int2 pos) : world(pWorld), xPos(pos.x), zPos(pos.y) {}
	int8_t GetHeightValue(uint8_t x, uint8_t z);
	void GenerateHeightMap();
	void PrintHeightmap();
	bool CanBlockSeeTheSky(Int3 pos);
	void SetLight(bool skyLight, Int3 pos, int8_t newLight);
	int8_t GetLight(bool skyLight, Int3 pos);
	int8_t GetTotalLight(Int3 pos);
	void ClearChunk();

	void SetBlockType(int8_t type, Int3 pos);
	int8_t GetBlockType(Int3 pos);
	void SetBlockMeta(int8_t meta, Int3 pos);
	int8_t GetBlockMeta(Int3 pos);
	void SetBlockTypeAndMeta(int8_t type, int8_t meta, Int3 pos);

	void SetBlockLight(int8_t value, Int3 pos);
	int8_t GetBlockLight(Int3 pos);
	void SetSkyLight(int8_t value, Int3 pos);
	int8_t GetSkyLight(Int3 pos);

	bool InChunkBounds(Int3 &pos);

	void AddTileEntity(std::unique_ptr<TileEntity> &&te);
	TileEntity *GetTileEntity(Int3 pos);
	std::vector<TileEntity *> GetTileEntities();
	std::vector<SignTile *> GetSigns();
	std::array<uint8_t, CHUNK_WIDTH_X * CHUNK_HEIGHT * CHUNK_WIDTH_Z> GetBlockTypes();
	std::array<uint8_t, CHUNK_WIDTH_X *(CHUNK_HEIGHT / 2) * CHUNK_WIDTH_Z> GetBlockMetas();
	std::array<uint8_t, CHUNK_WIDTH_X *(CHUNK_HEIGHT / 2) * CHUNK_WIDTH_Z> GetBlockLights();
	std::array<uint8_t, CHUNK_WIDTH_X *(CHUNK_HEIGHT / 2) * CHUNK_WIDTH_Z> GetSkyLights();
};