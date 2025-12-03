#include "generator.h"

/**
 * @brief The lua-based world generator
 * 
 */
class GeneratorLua : public Generator {
  public:
	GeneratorLua(int64_t seed, World *world);
	virtual ~GeneratorLua() {
		if (L) {
			lua_close(L);
		}
	}
	std::shared_ptr<Chunk> GenerateChunk(Int2 chunkPos) override;
	bool PopulateChunk(Int2 chunkPos) override;

  private:
	std::string name = GENERATOR_DEFAULT_NAME;
	int32_t apiVersion = GENERATOR_LATEST_VERSION;
	lua_State *L;

	void RegisterGlobals();
	static int64_t GetSeed(lua_State *L);

	// --- TERRAIN GEN RELATED ---
	const siv::PerlinNoise::seed_type seedp = 0;
	const siv::PerlinNoise perlin{seedp};

	static int lua_Index(lua_State *L);
	static int lua_Between(lua_State *L);
	static int lua_SpatialPRNG(lua_State *L);
	static int lua_GetNoiseWorley(lua_State *L);
	static int lua_GetNoisePerlin2D(lua_State *L);
	static int lua_GetNoisePerlin3D(lua_State *L);
	static int lua_GetNaturalGrass(lua_State *L);
	static int lua_CheckChunk(lua_State *L);
	static int lua_PlaceBlock(lua_State *L);
	static int lua_GetBlock(lua_State *L);
	static int lua_GetBiomeMap(lua_State *L);
};