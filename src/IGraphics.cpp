#include "IGraphics.hpp"
#include "Serializer.hpp"
#include "Actor.hpp"

#include <cassert>

const luaL_Reg IGraphics::METHODS[];

bool IGraphics::testBounds(float x, float y, float& xl, float& yl) const
{
    assert(m_actor != nullptr);

    if (!isVisible())
        return false;

    Aabb bounds = m_actor->getAabb();
    if (!bounds.isContaining(x, y))
        return false;

    xl = (x - bounds.getLeft()) / bounds.getWidth();
    yl = (y - bounds.getTop()) / bounds.getHeight();
    return true;
}

void IGraphics::construct(lua_State* L)
{
    getValueOpt(L, 2, "visible", m_isVisible);
    getListOpt(L, 2, "color", m_color.r, m_color.g, m_color.b);
}

void IGraphics::clone(lua_State* /*L*/, IGraphics* source)
{
    m_color = source->m_color;
    m_isVisible = source->m_isVisible;
}

void IGraphics::serialize(lua_State* /*L*/, Serializer* serializer, ObjectRef* ref)
{
    serializer->setList(ref, "", "color", m_color.r, m_color.g, m_color.b);
    serializer->setBoolean(ref, "", "visible", isVisible());
}

int IGraphics::script_setVisible(lua_State* L)
{
    // Validate function arguments
    IGraphics* self = IGraphics::checkInterface(L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    self->setVisible(lua_toboolean(L, 2) == 1);

    return 0;
}

int IGraphics::script_isVisible(lua_State* L)
{
    // Validate function arguments
    IGraphics* self = IGraphics::checkInterface(L, 1);

    lua_pushboolean(L, self->isVisible());

    return 1;
}

int IGraphics::script_setColor(lua_State* L)
{
    // Validate function arguments
    IGraphics* self = IGraphics::checkInterface(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    for (int i = 1; i <= 3; ++i)
        luaL_argcheck(L, (lua_rawgeti(L, 2, i) == LUA_TNUMBER), 2, "color should have three numbers {r, g, b}");

    float red = float(lua_tonumber(L, -3));
    float green = float(lua_tonumber(L, -2));
    float blue = float(lua_tonumber(L, -1));
    self->setColor(red, green, blue);

    return 0;
}
