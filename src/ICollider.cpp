#include "ICollider.hpp"
#include "Serializer.hpp"

const luaL_Reg ICollider::METHODS[];

void ICollider::construct(lua_State* L)
{
    getValueOpt(L, 2, "group", m_colliderGroup);
    getValueOpt(L, 2, "mask", m_colliderMask);
    getValueOpt(L, 2, "collidable", m_collidable);
}

void ICollider::clone(lua_State* L, ICollider* source)
{
    m_colliderGroup = source->m_colliderGroup;
    m_colliderMask = source->m_colliderMask;
    m_collidable = source->m_collidable;
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
