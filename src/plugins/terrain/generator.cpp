#include "generator.h"

// Prepare the Generator to utilize some preset numbers and functions
Generator::Generator(int64_t seed, World* world) {
    
}

Generator::~Generator() {

}

std::unique_ptr<Chunk> Generator::GenerateChunk(int32_t cX, int32_t cZ) {
    return nullptr;
}

bool Generator::PopulateChunk(int32_t cX, int32_t cZ) {
    return true;
}