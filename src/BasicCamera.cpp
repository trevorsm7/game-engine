#include "BasicCamera.hpp"
#include "IRenderer.hpp"
#include "Serializer.hpp"

#include "lua.hpp"

void BasicCamera::construct(lua_State* L, int index)
{
    const int relIndex = index < 0 ? index - 1 : index;

    lua_pushliteral(L, "size");
    if (lua_rawget(L, relIndex) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        m_width = luaL_checknumber(L, -2);
        m_height = luaL_checknumber(L, -1);
        m_center.x = m_width * 0.5f;
        m_center.y = m_height * 0.5f;
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    m_transform.setW(m_width);
    m_transform.setH(m_height);

    lua_pushliteral(L, "center");
    if (lua_rawget(L, relIndex) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        m_center.x = luaL_checknumber(L, -2);
        m_center.y = luaL_checknumber(L, -1);
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "fixed");
    if (lua_rawget(L, relIndex) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TBOOLEAN);
        m_fixed = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
}

void BasicCamera::serialize(lua_State* L, const char* table, Serializer* serializer, ObjectRef* ref) const
{
    float size[2] = {m_width, m_height};
    serializer->setArray(ref, table, "size", size, 2);

    float center[2] = {m_center.x, m_center.y};
    serializer->setArray(ref, table, "center", center, 2);

    //serializer->setBoolean(ref, table, "fixed", m_fixed);
    if (!m_fixed)
        serializer->setBoolean(ref, table, "fixed", false);
}

void BasicCamera::resize(int width, int height)
{
    if (m_fixed)
    {
        m_transform.setW(m_width);
        m_transform.setH(m_height);
    }
    else
    {
        float w = float(width);
        float h = float(height);

        if (m_height * w / h > m_width)
        {
            m_transform.setW(m_width);
            m_transform.setH(m_width * h / w);
        }
        else
        {
            m_transform.setW(m_height * w / h);
            m_transform.setH(m_height);
        }
    }

    m_transform.setX(m_center.x - m_transform.getW() * 0.5f);
    m_transform.setY(m_center.y - m_transform.getH() * 0.5f);
}

void BasicCamera::preRender(IRenderer* renderer)
{
    renderer->pushCameraTransform(m_transform);
}

void BasicCamera::postRender(IRenderer* renderer)
{
    renderer->popCameraTransform();
}

void BasicCamera::setCenter(float x, float y)
{
    m_center.x = x;
    m_center.y = y;

    m_transform.setX(x - m_transform.getW() * 0.5f);
    m_transform.setY(y - m_transform.getH() * 0.5f);
}

void BasicCamera::setOrigin(float x, float y)
{
    m_center.x = x + m_transform.getW() * 0.5f;
    m_center.y = y + m_transform.getH() * 0.5f;

    m_transform.setX(x);
    m_transform.setY(y);
}

void BasicCamera::mouseToWorld(const MouseEvent& event, float& x, float& y) const
{
    const float fractionX = float(event.x) / float(event.w);
    const float fractionY = float(event.y) / float(event.h);
    x = fractionX * m_transform.getW() + m_transform.getX();
    y = fractionY * m_transform.getH() + m_transform.getY();
}
