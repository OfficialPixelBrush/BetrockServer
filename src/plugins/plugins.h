#pragma once
#include <string>
#include <iostream>
#include <filesystem>
#include <lua.hpp>
#include <stdexcept>

#include "logger.h"
#include "luahelper.h"

#define PLUGIN_DEFAULT_NAME "Plugin"
#define PLUGIN_LATEST_VERSION 1

class Plugin {
    public:
        Plugin(std::string path);
        ~Plugin() {
            if (L) {
                lua_close(L);
            }
        }
        std::string GetName();
        int32_t GetApiVersion();
    private:
        Betrock::Logger* logger;
        std::string name = PLUGIN_DEFAULT_NAME;
        int32_t apiVersion = PLUGIN_LATEST_VERSION;
        lua_State* L;

        // --- Lua Bindings Functions ---
        int lua_GlobalChat(lua_State *L);
        int lua_Chat(lua_State *L);
};