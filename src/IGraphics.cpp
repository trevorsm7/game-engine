#include "IGraphics.hpp"
#include "Serializer.hpp"

const luaL_Reg IGraphics::METHODS[];

void IGraphics::construct(lua_State* L)
{
    lua_pushliteral(L, "visible");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TBOOLEAN);
        setVisible(lua_toboolean(L, -1));
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "color");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        // TODO refactor w/ script_setColor
        for (int i = 1; i <= 3; ++i)
            luaL_argcheck(L, (lua_rawgeti(L, -i, i) == LUA_TNUMBER), 1, "color = {r, g, b} must be numbers");
        float r = lua_tonumber(L, -3);
        float g = lua_tonumber(L, -2);
        float b = lua_tonumber(L, -1);
        setColor(r, g, b);
        lua_pop(L, 3);
    }
    lua_pop(L, 1);
}

void IGraphics::clone(lua_State* L, IGraphics* source)
{
    m_color = source->m_color;
    m_isVisible = source->m_isVisible;
}

void IGraphics::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    serializer->setArray(ref, "", "color", &(m_color.r), 3);
    serializer->setBoolean(ref, "", "visible", isVisible());
}

int IGraphics::script_setVisible(lua_State* L)
{
    // Validate function arguments
    IGraphics* self = IGraphics::checkInterface(L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    self->setVisible(lua_toboolean(L, 2));

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

    float red = lua_tonumber(L, -3);
    float green = lua_tonumber(L, -2);
    float blue = lua_tonumber(L, -1);
    self->setColor(red, green, blue);

    return 0;
}
