#include "plugins.h"

// Initialize a newly loaded plugin
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
	if (!lua_isnumber(L, -1)) {
		lua_close(L);
		throw std::runtime_error("Invalid PluginApiVersion!");
	} else {
		apiVersion = (int32_t)lua_tonumber(L, -1);
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
}

std::string Plugin::GetName() { return this->name; }
int32_t Plugin::GetApiVersion() { return this->apiVersion; }