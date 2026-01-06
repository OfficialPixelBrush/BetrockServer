#include "client.h"

#include "server.h"

// Check if the player has moved far enough to warrant sending new chunk data
bool Client::CheckIfNewChunksRequired() {
	Vec3 lastPos = lastChunkUpdatePosition;
	Vec3 newPos = player->position;
	// Remove vertical component
	lastPos.y = 0;
	newPos.y = 0;
	if (GetEuclidianDistance(lastPos,newPos) > 16) {
		return true;
	}
	return false;
}

// Check if a chunk already exists in memory or if it needs to be loaded from memory first
void Client::ProcessChunk(const Int2& position, WorldManager* wm) {
	// TODO: This is awful to do for every chunk :(
    // Skip processing if chunk is already visible
    if (std::find(visibleChunks.begin(), visibleChunks.end(), position) != visibleChunks.end()) {
        return;
    }

    // Check if the chunk has already been put into the queue
	if (!wm->world.ChunkExists(position) || 
		!wm->world.IsChunkPopulated(position)) {
		wm->AddChunkToQueue(position, shared_from_this());
	} else {
		AddNewChunk(position);
	}
	
    // Check if the chunk has already been populated
	/*
    if (wm->world.IsChunkPopulated(position.x,position.z)) {
		// Otherwise queue chunk loading or generation
		AddNewChunk(position);
    }
		*/
}

// Figure out what chunks the player can see and
// add them to the NewChunks queue if any new ones are added
void Client::DetermineVisibleChunks(bool forcePlayerAsCenter) {
    auto &server = Betrock::Server::Instance();

    Int3 centerPos;
	if (forcePlayerAsCenter) {
		centerPos = Vec3ToInt3(player->position);
	} else {
    	Vec3 delta = player->position - lastChunkUpdatePosition;
		centerPos = Vec3ToInt3(player->position+delta);
	}
    Int3 playerChunkPos = BlockToChunkPosition(centerPos);
    int32_t pX = playerChunkPos.x;
    int32_t pZ = playerChunkPos.z;

    auto chunkDistance = server.GetChunkDistance();

    // Remove chunks that are out of range
    for (auto it = visibleChunks.begin(); it != visibleChunks.end(); ) {
        int32_t distanceX = abs(pX - it->x);
        int32_t distanceZ = abs(pZ - it->y);
        if (distanceX > chunkDistance || distanceZ > chunkDistance) {
            // Tell client chunk is no longer visible
            Respond::PreChunk(response, Int2{it->x, it->y}, 0);
            it = visibleChunks.erase(it);
        } else {
            ++it;
        }
    }

    auto wm = server.GetWorldManager(player->dimension);

	// Iterate over all chunks within a bounding box defined by chunkDistance
	for (int32_t r = 0; r < chunkDistance; r++) {
		// Top and Bottom rows
		for (int32_t x = -r; x <= r; x++) {
			for (int32_t z : {-r, r}) {
				Int2 position = Int2{x+pX, z+pZ};
				ProcessChunk(position, wm);
			}
		}
		// Left and Right columns (excluding corners to avoid duplicates)
		for (int32_t z = -r + 1; z <= r - 1; z++) {
			for (int32_t x : {-r, r}) {
				Int2 position = Int2{x+pX, z+pZ};
				ProcessChunk(position, wm);
			}
		}
	}

    lastChunkUpdatePosition = player->position;
}

// Send the chunks from the newChunks queue to the player
void Client::SendNewChunks() {
	// Send chunks in batches of 10
	// TODO: Dynamically size this based on remaining space in response
	int32_t sentThisCycle = 10;
	auto wm = Betrock::Server::Instance().GetWorldManager(player->dimension);
	std::unique_lock<std::mutex> lock(newChunksMutex, std::try_to_lock);
	if (!lock.owns_lock()) {
		return;
	}

	for (auto it = newChunks.begin(); it != newChunks.end() && sentThisCycle > 0; ) {
        // Skip chunks that aren't fully populated yet
        if (!wm->world.IsChunkPopulated(Int2{it->x, it->y})) {
            ++it; // move to next chunk
            continue;
        }

        uint8_t chunkData[CHUNK_DATA_SIZE];
        wm->world.GetChunkData(chunkData, *it);

        auto signs = wm->world.GetChunkSigns(*it);

        // Compress and send chunk
        size_t compressedSize = 0;
        auto chunk = CompressChunk(chunkData, compressedSize);
        if (chunk) {
            visibleChunks.push_back(Int2{it->x, it->y});

            Respond::PreChunk(response, Int2{it->x, it->y}, 1);
            Respond::Chunk(
                response,
                Int3{it->x << 4, 0, it->y << 4},
                CHUNK_WIDTH_X - 1,
                CHUNK_HEIGHT - 1,
                CHUNK_WIDTH_Z - 1,
                compressedSize,
                chunk.get()
            );

            for (auto& s : signs) {
                Respond::UpdateSign(response, s->position, s->lines);
            }
        }

        // Remove the chunk from the queue and decrement send counter
        it = newChunks.erase(it);
        --sentThisCycle;
    }
    newChunks.shrink_to_fit();
}
