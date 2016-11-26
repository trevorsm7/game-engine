#include "Transform.hpp"
#include "Serializer.hpp"
#include "IUserdata.hpp"

#include "lua.hpp"

void Transform::construct(lua_State* L, int index)
{
    const int relIndex = index < 0 ? index - 1 : index;
    IUserdata::getListOpt(L, relIndex, "position", m_x, m_y);
    IUserdata::getListOpt(L, relIndex, "scale", m_sx, m_sy);
}

void Transform::serialize(lua_State* L, const char* table, Serializer* serializer, ObjectRef* ref) const
{
    serializer->setList(ref, table, "position", m_x, m_y);
    serializer->setList(ref, table, "scale", m_sx, m_sy);
}
