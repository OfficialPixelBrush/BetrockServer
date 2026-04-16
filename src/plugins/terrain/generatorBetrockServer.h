#include "generator.h"
#include "blockHelper.h"

/**
 * @brief A custom world generator
 * 
 */
class GeneratorBetrockServer : public Generator {
  public:
	GeneratorBetrockServer(int64_t seed, World *world);
	~GeneratorBetrockServer() = default;
	std::shared_ptr<Chunk> GenerateChunk(Int2 chunkPos) override;
	bool PopulateChunk(Int2 chunkPos) override;
};