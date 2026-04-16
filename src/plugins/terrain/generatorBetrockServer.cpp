#include "blocks.h"
#include "datatypes.h"
#include "javaMath.h"
#include "generatorBetrockServer.h"

/**
 * @brief Construct a new BetrockServer Generator
 * 
 * @param pSeed The seed of the generated world
 * @param pWorld The world that the generator belongs to
 */
GeneratorBetrockServer::GeneratorBetrockServer(int64_t pSeed, World *pWorld) : Generator(pSeed, pWorld) {
	logger = &Betrock::Logger::Instance();
	this->seed = pSeed;
	this->world = pWorld;
}

/**
 * @brief Generate a non-populated chunk
 * 
 * @param chunkPos The x,z coordinate of the chunk
 * @return std::shared_ptr<Chunk> 
 */
std::shared_ptr<Chunk> GeneratorBetrockServer::GenerateChunk(Int2 chunkPos) {
	std::shared_ptr<Chunk> c = std::make_shared<Chunk>(this->world, chunkPos);
	c->state = ChunkState::Generating;

	// Allocate empty chunk
	c->ClearChunk();

    for (int x = 0; x < CHUNK_WIDTH_X; x++) {
        for (int z = 0; z < CHUNK_WIDTH_Z; z++) {
            double hillRange = 0.2 + GetNoisePerlin2D(0, Vec3{
                double(chunkPos.x * CHUNK_WIDTH_X + x)*0.006,
                0.0,
                double(chunkPos.y * CHUNK_WIDTH_Z + z)*0.006
            },2)*0.8;

            // Mountain value
            double value = lerp(hillRange, 0.2 + GetNoisePerlin2D(0, Vec3{
                double(chunkPos.x * CHUNK_WIDTH_X + x)*0.01,
                0.0,
                double(chunkPos.y * CHUNK_WIDTH_Z + z)*0.01
            },8)*0.8, 0.5);

            //std::cout << value << std::endl;
            for (int y = 0; y < CHUNK_HEIGHT; y++) {
                c->SetBlockType(BLOCK_STONE, Int3{x,y,z});
                if (value < double(y)/double(CHUNK_HEIGHT))
                    break;
            }
            for (int y = WATER_LEVEL; y >= 0; y--) {
                if (c->GetBlockType(Int3{x,y,z}) > BLOCK_AIR)
                    break;
                c->SetBlockType(BLOCK_WATER_STILL, Int3{x,y,z});
            }
        }
    }

    c->GenerateHeightMap();    

	c->state = ChunkState::Generated;
	c->modified = true;
	return c;
}

/**
 * @brief Populates the specified chunk
 * 
 * @param chunkPos The x,z coordinate of the chunk
 * @return True if population succeeded
 */
bool GeneratorBetrockServer::PopulateChunk( [[maybe_unused]] Int2 chunkPos) {
    return true;
}