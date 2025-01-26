#pragma once
#include <lua.hpp>
#include <iostream>

bool CheckLua(lua_State *L, int r);
bool CheckInt3(lua_State *L, int startIndex = 1);