#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__

#include "lua.hpp"

class Serializer
{
public:
    int indent;
    Serializer(): indent(0) {}

    void serialize(lua_State* L, int index);
    void serializeUserdata(lua_State* L, int index);

    static Serializer* checkSerializer(lua_State* L, int index)
    {
        luaL_checktype(L, index, LUA_TLIGHTUSERDATA);
        return reinterpret_cast<Serializer*>(lua_touserdata(L, index));
    }
};

#endif
