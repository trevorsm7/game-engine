#include "ICollider.hpp"
#include "Serializer.hpp"

const luaL_Reg ICollider::METHODS[];

void ICollider::construct(lua_State* L)
{
    lua_pushliteral(L, "group");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TNUMBER);
        setGroup(lua_tointeger(L, -1));
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "mask");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TNUMBER);
        setMask(lua_tointeger(L, -1));
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "collidable");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TBOOLEAN);
        setCollidable(lua_toboolean(L, -1));
    }
    lua_pop(L, 1);
}

void ICollider::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    serializer->setNumber(ref, "", "group", m_colliderGroup);
    serializer->setNumber(ref, "", "mask", m_colliderMask); // TODO option to print in hex?
    serializer->setBoolean(ref, "", "collidable", m_collidable);
}

int ICollider::script_setCollidable(lua_State* L)
{
    // Validate function arguments
    ICollider* self = ICollider::checkInterface(L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    self->setCollidable(lua_toboolean(L, 2));

    return 0;
}
