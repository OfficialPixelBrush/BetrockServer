#pragma once
#include <lua.hpp>
#include <iostream>

#include "logger.h"
#include "datatypes.h"
#include "helper.h"

bool CheckLua(lua_State *L, int32_t r);
bool CheckNum3(lua_State *L, int32_t startIndex = 1);
bool CheckNum2(lua_State *L, int32_t startIndex = 1);
Block DecodeBlock(lua_State *L);