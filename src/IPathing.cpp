#include "IPathing.hpp"

const luaL_Reg IPathing::METHODS[];

int IPathing::script_findPath(lua_State* L)
{
    // Validate function arguments
    IPathing* pathing = IPathing::checkInterface(L, 1);
    /*float x1 = static_cast<float>(luaL_checknumber(L, 2));
    float y1 = static_cast<float>(luaL_checknumber(L, 3));
    float x2 = static_cast<float>(luaL_checknumber(L, 4));
    float y2 = static_cast<float>(luaL_checknumber(L, 5));*/
    const int x1 = luaL_checkinteger(L, 2);
    const int y1 = luaL_checkinteger(L, 3);
    const int x2 = luaL_checkinteger(L, 4);
    const int y2 = luaL_checkinteger(L, 5);

    int xOut, yOut;
    if (pathing->findPath(x1, y1, x2, y2, xOut, yOut))
    {
        lua_pushinteger(L, xOut);
        lua_pushinteger(L, yOut);
        return 2;
    }

    return 0;
}

int IPathing::script_clearPath(lua_State* L)
{
    // Validate function arguments
    IPathing* pathing = IPathing::checkInterface(L, 1);

    pathing->update(0);

    return 0;
}
