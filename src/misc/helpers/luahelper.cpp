#include "luahelper.h"

// Check if the current lua state is valid
bool CheckLua(lua_State *L, int r) {
    if (r != LUA_OK)  {
        std::string errormsg = lua_tostring(L,-1);
        Betrock::Logger::Instance().Info(errormsg);
        return false;
    }
    return true;
}

// Check if 3 Numbers are passed
bool CheckNum3(lua_State *L, int startIndex) {
    if (!lua_isnumber(L, startIndex+0) || !lua_isnumber(L, startIndex+1) || !lua_isnumber(L, startIndex+2)) {
        luaL_error(L, "Expected three numeric arguments");
        return false; // Return 0 since we raised an error
    }
    return true;
}

// Check if 2 Numbers are passed
bool CheckNum2(lua_State *L, int startIndex) {
    if (!lua_isnumber(L, startIndex+0) || !lua_isnumber(L, startIndex+1)) {
        luaL_error(L, "Expected two numeric arguments");
        return false; // Return 0 since we raised an error
    }
    return true;
}