#include "plugins.h"
Plugin::Plugin(std::string path) {
    L = luaL_newstate();
    luaL_openlibs(L);

    // Load passed path
    if (luaL_dofile(L, path.c_str())) {
        std::cerr << "Error: " << lua_tostring(L, -1) << std::endl;
        lua_close(L);
        throw std::runtime_error("Error!");
    }

    // Load the plugins name
    lua_getglobal(L, "PluginName");
    if (!lua_isstring(L,-1)) {
        std::cerr << "Invalid PluginName!" << std::endl;
        throw std::runtime_error("Error!");
    } else {
        name = std::string(lua_tostring(L,-1));
    }

    // Load the plugins API version
    lua_getglobal(L, "PluginApiVersion");
    if (!lua_isnumber(L,-1)) {
        std::cerr << "Invalid PluginApiVersion!" << std::endl;
        throw std::runtime_error("Error!");
    } else {
        apiVersion = (int32_t)lua_tonumber(L,-1);
    }

    // Init Function
    lua_getglobal(L, "Init");
    if (!lua_isfunction(L,-1)) {
        std::cerr << "Init was not found!" << std::endl;
        throw std::runtime_error("OnStart was not found!");
    } else {
        CheckLua(L,lua_pcall(L,0,0,0));
    }
}

std::string Plugin::GetName() { return this->name; }
int32_t Plugin::GetApiVersion() { return this->apiVersion; }