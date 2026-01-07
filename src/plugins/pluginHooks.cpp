#include "plugin.h"

/**
 * @brief Run if a block was broken
 * 
 * @param L 
 * @return int32_t 
 */
int32_t Plugin::BlockBreakHook() {
	lua_getglobal(L, "blockBreakHook");
	if (lua_isfunction(L, -1)) {
		CheckLua(L, lua_pcall(L, 0, 0, 0));
	}
    return 1;
}

/**
 * @brief Run if a block was placed
 * 
 * @param L 
 * @return int32_t 
 */
int32_t Plugin::BlockPlaceHook() {
	lua_getglobal(L, "blockPlaceHook");
	if (lua_isfunction(L, -1)) {
		CheckLua(L, lua_pcall(L, 0, 0, 0));
	}
    return 1;
}