#pragma once

#include "helper.h"
#include "items.h"
#include "config.h"
#include <lua.hpp>
#include "luahelper.h"
#include "lighting.h"

class Generator {
    private:
        lua_State* L;
        virtual Block GenerateBlock(Int3 position, int8_t blocksSinceSkyVisible = 0);
    public:
        Generator();
        int64_t seed;
        virtual Chunk GenerateChunk(int32_t cX, int32_t cZ);
};