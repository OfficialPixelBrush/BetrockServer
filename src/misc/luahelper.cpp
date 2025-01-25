#include "luahelper.h"

bool CheckLua(lua_State *L, int r) {
    if (r != LUA_OK)  {
        std::string errormsg = lua_tostring(L,-1);
        std::cerr << errormsg << std::endl;
        return false;
    }
    return true;
}