#include "worldManager.h"

void WorldManager::AddChunkToQueue(int32_t x, int32_t z, Player* requestPlayer) {
    std::lock_guard<std::mutex> lock(queueMutex);  // Lock the mutex to ensure thread-safety
    chunkQueue.push(QueueChunk(Int3{x,0,z},requestPlayer));
}

void WorldManager::GenerateQueuedChunks() {
    std::lock_guard<std::mutex> lock(queueMutex);
    while(!chunkQueue.empty()) {
        //std::cout << "Generating Chunks: " << chunkQueue.size() << std::endl;
        QueueChunk qc = chunkQueue.front();
        Chunk c = generator.GenerateChunk(qc.position.x,qc.position.z);
        world.AddChunk(qc.position.x,qc.position.z,c);

        // Send the new chunk data out
        if (qc.requestPlayer != nullptr) {
            std::vector<uint8_t> response;
            auto chunkData = world.GetChunkData(qc.position);
			if (!chunkData) {
				continue;
			}
			size_t compressedSize = 0;
			char* chunk = CompressChunk(chunkData.get(), compressedSize);
			
			if (chunk) {
				Respond::PreChunk(response, qc.position.x, qc.position.z, 1);

				// Track newly visible chunks
				qc.requestPlayer->visibleChunks.push_back(qc.position);

				// Send compressed chunk data
				Respond::Chunk(
					response, 
					Int3{qc.position.x << 4, 0, qc.position.z << 4}, 
					CHUNK_WIDTH_X - 1, 
					CHUNK_HEIGHT  - 1, 
					CHUNK_WIDTH_Z - 1, 
					compressedSize, 
					chunk
				);
			}
			delete [] chunk;
            SendToPlayer(response,qc.requestPlayer);
        }
        chunkQueue.pop();
    }
}

bool WorldManager::QueueIsEmpty() {
    return chunkQueue.empty();
}

void WorldManager::SetSeed(int64_t seed) {
    this->seed = seed;
    world.seed = seed;
    generator.PrepareGenerator(seed);
}

int64_t WorldManager::GetSeed() {
    return this->seed;
}

void WorldManager::Run() {
    while(alive) {
        GenerateQueuedChunks();
    }
}

void WorldManager::SetName(std::string name) {
    this->name = name;
}

std::string WorldManager::GetName() {
    return this->name;
}

std::string ConvertIndexIntoExtra(int8_t worldId) {
    if (worldId == 0) {
        return "";
    }
    return "DIM" + std::to_string(worldId);
}