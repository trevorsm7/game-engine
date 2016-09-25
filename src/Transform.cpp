#include "Transform.hpp"
#include "Serializer.hpp"

#include "lua.hpp"

void Transform::construct(lua_State* L, int index)
{
    const int relIndex = index < 0 ? index - 1 : index;
    lua_pushliteral(L, "position");
    if (lua_rawget(L, relIndex) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        m_x = luaL_checknumber(L, -2);
        m_y = luaL_checknumber(L, -1);
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "scale");
    if (lua_rawget(L, relIndex) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        // TODO let graphics/collider define the base size and use this as a scaling factor
        m_w = luaL_checknumber(L, -2);
        m_h = luaL_checknumber(L, -1);
        lua_pop(L, 2);
    }
    lua_pop(L, 1);
}

void Transform::serialize(lua_State* L, const char* table, ObjectRef* ref) const
{
    float position[2] = {m_x, m_y};
    ref->setArray(table, "position", position, 2);

    float scale[2] = {m_w, m_h};
    ref->setArray(table, "scale", scale, 2);
}
