#include "luahelper.h"

bool CheckLua(lua_State *L, int r) {
    if (r != LUA_OK)  {
        std::string errormsg = lua_tostring(L,-1);
        std::cerr << errormsg << std::endl;
        return false;
    }
    return true;
}

bool CheckInt3(lua_State *L, int startIndex) {
    if (!lua_isnumber(L, startIndex+0) || !lua_isnumber(L, startIndex+1) || !lua_isnumber(L, startIndex+2)) {
        luaL_error(L, "Expected three numeric arguments");
        return false; // Return 0 since we raised an error
    }
    return true;
}