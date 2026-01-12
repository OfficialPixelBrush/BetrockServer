#include "plugin.h"
#include "server.h"

/**
 * @brief Initialize a newly loaded plugin
 * 
 * @param path Path from which to load the plugins .lua file
 */
Plugin::Plugin(std::string path) {
	logger = &Betrock::Logger::Instance();
	L = luaL_newstate();
	luaL_openlibs(L);

	// Load passed path
	if (luaL_dofile(L, path.c_str())) {
		lua_close(L);
		throw std::runtime_error(lua_tostring(L, -1));
	}

	// Load the plugins name
	lua_getglobal(L, "PluginName");
	if (!lua_isstring(L, -1)) {
		logger->Warning("Invalid PluginName!");
	} else {
		name = std::string(lua_tostring(L, -1));
	}

	// Load the plugins API version
	lua_getglobal(L, "PluginApiVersion");
	if (!lua_isinteger(L, -1)) {
		lua_close(L);
		throw std::runtime_error("Invalid PluginApiVersion!");
	} else {
		apiVersion = int32_t(lua_tointeger(L, -1));
	}

	if (apiVersion > PLUGIN_LATEST_VERSION) {
		lua_close(L);
		throw std::runtime_error("\"" + name + "\" was made for a newer version of BetrockServer!");
	}

	// Init Function
	lua_getglobal(L, "Init");
	if (!lua_isfunction(L, -1)) {
		logger->Warning("Init was not found!");
		// throw std::runtime_error("OnStart was not found!");
	} else {
		CheckLua(L, lua_pcall(L, 0, 0, 0));
	}

	// Check for hooks
	lua_getglobal(L, "blockPlaceHook");
	if (lua_isfunction(L, -1)) {
		// Add hook
	}
	lua_pop(L, 1);
	lua_getglobal(L, "blockBreakHook");
	if (lua_isfunction(L, -1)) {
		// Add hook
	}
	lua_pop(L, 1);

	// Register Lua API
	lua_register(L, "globalChat", Plugin::lua_GlobalChat);
	lua_register(L, "getPlayerList", Plugin::lua_GetPlayerList);
	lua_register(L, "getBlock", Plugin::lua_GetBlock);
	lua_register(L, "setBlock", Plugin::lua_SetBlock);
}

/**
 * @brief Gets the name of the plugin
 * 
 * @return Plugin name
 */
std::string Plugin::GetName() { return this->name; }

/**
 * @brief Get the version integer the plugin has
 * 
 * @return Version Integer
 */
int32_t Plugin::GetApiVersion() { return this->apiVersion; }