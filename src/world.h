#include <libdeflate.h>
#include <iostream>
#include <vector>

std::vector<uint8_t> GetChunk(int32_t x, int8_t y, int32_t z);
char* CompressChunk(std::vector<uint8_t> chunk, size_t &compressed_size);