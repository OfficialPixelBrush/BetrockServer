#include "worldManager.h"

QueueChunk::QueueChunk(Int3 position, Player* requestPlayer) {
    this->position = position;
    AddPlayer(requestPlayer);
}

void QueueChunk::AddPlayer(Player* requestPlayer) {
    requestedPlayers.push_back(requestPlayer);
}

void WorldManager::AddChunkToQueue(int32_t x, int32_t z, Player* requestPlayer) {
    std::lock_guard<std::mutex> lock(queueMutex);  // Lock the mutex to ensure thread-safety

    auto hash = GetChunkHash(x, z);  // Compute the hash for the chunk
    auto it = chunkQueue.find(hash); // Check if the key exists in the map

    if (it != chunkQueue.end()) {
        // The key exists, so add the player to the existing chunk
        it->second.AddPlayer(requestPlayer);
    } else {
        // The key doesn't exist, so create a new QueueChunk and insert it
        chunkQueue[hash] = QueueChunk(Int3{x, 0, z}, requestPlayer);
    }
}

void WorldManager::GenerateQueuedChunks() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if(!chunkQueue.empty()) {
        for (auto it = chunkQueue.begin(); it != chunkQueue.end(); ) {
            int64_t key = it->first;
            QueueChunk cq = it->second;
            Int3 pos = cq.position;

            Chunk c = generator.GenerateChunk(pos.x,pos.z);
            world.AddChunk(pos.x,pos.z,c);

            // If no players need this chunk (for some reason)
            // just move on and get to the next chunk
            if (cq.requestedPlayers.empty()) {
                continue;
            }

            // Get the new chunk and compress it
            std::vector<uint8_t> response;
            auto chunkData = world.GetChunkData(pos);
            if (!chunkData) {
                it = chunkQueue.erase(it);
                continue;
            }
            size_t compressedSize = 0;
            char* chunk = CompressChunk(chunkData.get(), compressedSize);
            
            // If this worked, get moving
            if (chunk) {
                // Send the new chunk data out
                for (Player* p : cq.requestedPlayers) {
                    if (!p) {
                        continue;
                    }
                    Respond::PreChunk(response, pos.x, pos.z, 1);

                    // Track newly visible chunks
                    p->visibleChunks.push_back(pos);

                    // Send compressed chunk data
                    Respond::Chunk(
                        response, 
                        Int3{pos.x << 4, 0, pos.z << 4}, 
                        CHUNK_WIDTH_X - 1, 
                        CHUNK_HEIGHT  - 1, 
                        CHUNK_WIDTH_Z - 1, 
                        compressedSize, 
                        chunk
                    );
                    SendToPlayer(response,p);
                }
            }
            delete [] chunk;
            it = chunkQueue.erase(it);
        }
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
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    // make this function sleep for 1/2 second to prevent it running too often.
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