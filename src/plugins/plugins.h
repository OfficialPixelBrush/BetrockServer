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
    private:
        std::string name = DEFAULT_NAME;
        int32_t apiVersion = LATEST_VERSION;
        lua_State* L;
    public:
        Plugin(std::string path);
        ~Plugin() {
            if (L) {
                lua_close(L);
            }
        }
        std::string GetName();
        int32_t GetApiVersion();
};