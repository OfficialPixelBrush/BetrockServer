#include "generator.h"

// Prepare the Generator to utilize some preset numbers and functions
Generator::Generator([[maybe_unused]] int64_t pSeed, [[maybe_unused]] World *pWorld) {}

Generator::~Generator() {}

std::shared_ptr<Chunk> Generator::GenerateChunk([[maybe_unused]] Int2 chunkPos) {
	return nullptr;
}

bool Generator::PopulateChunk([[maybe_unused]] Int2 chunkPos) { return true; }