#pragma once
#include <filesystem>
#include <iostream>
#include <lua.hpp>
#include <stdexcept>
#include <string>

#include "logger.h"
#include "luahelper.h"

#define PLUGIN_DEFAULT_NAME "Plugin"
#define PLUGIN_LATEST_VERSION 2

/**
 * @brief A Plugin with its own dedicated Lua VM
 * 
 */
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
	int32_t BlockBreakHook();
	int32_t BlockPlaceHook();

  private:
	Betrock::Logger *logger;
	std::string name = PLUGIN_DEFAULT_NAME;
	int32_t apiVersion = PLUGIN_LATEST_VERSION;
	lua_State *L;

	// --- Lua Bindings Functions ---
	static int32_t lua_GlobalChat(lua_State *L);
	static int32_t lua_Chat(lua_State *L);
	static int32_t lua_GetPlayerList(lua_State *L);
	static int32_t lua_GetPlayer(lua_State *L);
	static int32_t lua_GetBlock(lua_State *L);
	static int32_t lua_SetBlock(lua_State *L);
};