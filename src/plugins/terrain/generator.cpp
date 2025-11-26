#include "generator.h"

// Prepare the Generator to utilize some preset numbers and functions
Generator::Generator( [[maybe_unused]] int64_t seed, [[maybe_unused]] World* world) {
    
}

Generator::~Generator() {

}

std::shared_ptr<Chunk> Generator::GenerateChunk( [[maybe_unused]] int32_t cX, [[maybe_unused]] int32_t cZ) {
    return nullptr;
}

bool Generator::PopulateChunk( [[maybe_unused]] int32_t cX, [[maybe_unused]] int32_t cZ) {
    return true;
}