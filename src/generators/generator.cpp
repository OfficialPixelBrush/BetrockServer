#include "generator.h"

Generator::Generator() {
    L = luaL_newstate();
    luaL_openlibs(L);
    // Execute a Lua script
    if (luaL_dofile(L, ("scripts/" + properties["generator"]).c_str())) {
        std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
        lua_close(L);
    }
}

Block Generator::GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible) {
    Block b;
    if (!L) {
        return b;
    }
    lua_getglobal(L, "GenerateBlock");
    if (!lua_isfunction(L,-1)) {
        std::cerr << "GenerateBlock was not found!" << std::endl;
        return b;
    }
    lua_pushnumber(L,position.x);
    lua_pushnumber(L,position.y);
    lua_pushnumber(L,position.z);
    lua_pushnumber(L,blocksSinceSkyVisible);
    if (CheckLua(L,lua_pcall(L,4,1,0))) {
        b.type = (int)lua_tonumber(L, -1);
    }
    lua_pop(L,1);
    return b;
}

Chunk Generator::GenerateChunk(int32_t cX, int32_t cZ) {
    Chunk c = Chunk();
    for (uint8_t x = 0; x < CHUNK_WIDTH_X; x++) {
        for (uint8_t z = 0; z < CHUNK_WIDTH_X; z++) {
            int8_t blocksSinceSkyVisible = 0;
            for (int8_t y = CHUNK_HEIGHT - 1; y >= 0; --y) {
                Int3 blockPos = Int3 {x,y,z};
                Int3 chunkPos = Int3 {cX,0,cZ};
                Int3 globalPos = LocalToGlobalPosition(chunkPos,blockPos);
                Block b = GenerateBlock(globalPos,blocksSinceSkyVisible);
                if (b.type > BLOCK_AIR) {
                    blocksSinceSkyVisible++;
                }
                
                c.blocks[GetBlockIndex(blockPos)] = b;
            }
        }   
    }
    CalculateChunkLight(&c);
    return c;
}