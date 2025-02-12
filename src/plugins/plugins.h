#pragma once
#include <string>
#include <iostream>
#include <filesystem>
#include <lua.hpp>
#include <stdexcept>

#include "luahelper.h"

#define DEFAULT_NAME "Plugin"
#define LATEST_VERSION 1

class Plugin {
    public:
        Plugin(std::string path);
        std::string GetName();
        int32_t GetApiVersion();
        lua_State* GetLuaState();
    private:
        std::string name = DEFAULT_NAME;
        int32_t apiVersion = LATEST_VERSION;
        lua_State* L;
};