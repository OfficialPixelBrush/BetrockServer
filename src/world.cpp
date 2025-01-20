#include "world.h"

#include <fstream>
#include <iterator>
#include <string>
#include <vector>

std::vector<uint8_t> GetChunk(int32_t x, int8_t y, int32_t z) {
    std::vector<uint8_t> bytes;
    // BlockData
    for (int cX = 0; cX < 16; cX++) {
        for (int cZ = 0; cZ < 16; cZ++) {
            for (int cY = 0; cY < 128; cY++) {
                if (cY == 0) {
                    bytes.push_back(2);
                } else {
                    bytes.push_back(0);
                }
                /*
                if (cY == 60) {
                    bytes.push_back(2);
                } else if (cY == 0) {
                    bytes.push_back(7);
                } else if (cY == 59) {
                    bytes.push_back(3);
                } else if (cY < 59) {
                    bytes.push_back(1);
                } else {
                    bytes.push_back(0);
                }*/
            }
        }
    }

    // Block Metadata
    for (int8_t cX = 0; cX < 16; cX++) {
        for (int8_t cZ = 0; cZ < 16; cZ++) {
            for (int8_t cY = 0; cY < (128/2); cY++) {
                 bytes.push_back(0);
            }
        }
    }

    // Block Light
    for (int8_t cX = 0; cX < 16; cX++) {
        for (int8_t cZ = 0; cZ < 16; cZ++) {
            for (int8_t cY = 0; cY < (128/2); cY++) {
                 bytes.push_back(0xff);
            }
        }
    }

    // Sky Light
    for (int8_t cX = 0; cX < 16; cX++) {
        for (int8_t cZ = 0; cZ < 16; cZ++) {
            for (int8_t cY = 0; cY < (128/2); cY++) {
                bytes.push_back(0xff);
            }
        }
    }
    return bytes;
}

char* CompressChunk(std::vector<uint8_t> chunk, size_t &compressed_size) {
    size_t chunkSize = chunk.size();

    // Create a compression context
    struct libdeflate_compressor *compressor = libdeflate_alloc_compressor(9);
    if (!compressor) {
        std::cerr << "Failed to allocate compressor\n";
        return nullptr;
    }

    // Allocate space for compressed data
    size_t max_compressed_size = libdeflate_zlib_compress_bound(compressor, chunkSize);
    char *compressed_data = new char[max_compressed_size];

    // Compress the data
    compressed_size = libdeflate_zlib_compress(compressor, chunk.data(), chunkSize,
                                                      compressed_data, max_compressed_size);

    if (compressed_size == 0) {
        std::cerr << "Compression failed\n";
        libdeflate_free_compressor(compressor);
        return nullptr;
    }
    libdeflate_free_compressor(compressor);

    return compressed_data;
}