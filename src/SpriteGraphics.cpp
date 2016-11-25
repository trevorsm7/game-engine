#include "SpriteGraphics.hpp"
#include "Serializer.hpp"
#include "IRenderer.hpp"
#include "Actor.hpp"

#include <cassert>

const luaL_Reg SpriteGraphics::METHODS[];

void SpriteGraphics::render(IRenderer* renderer)
{
    if (!isVisible())
        return;

    renderer->setColor(m_color.r, m_color.g, m_color.b);
    renderer->drawSprite(m_filename);
}

bool SpriteGraphics::testBounds(float x, float y) const
{
    assert(m_actor != nullptr);

    if (!isVisible())
        return false;

    Aabb bounds = m_actor->getAabb();
    return bounds.isContaining(x, y);
}

void SpriteGraphics::construct(lua_State* L)
{
    lua_pushliteral(L, "sprite");
    luaL_argcheck(L, (lua_rawget(L, 2) == LUA_TSTRING), 2, "{sprite = filename} is required");
    m_filename = lua_tostring(L, -1);
    lua_pop(L, 1);
}

void SpriteGraphics::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    serializer->setString(ref, "", "sprite", m_filename);
}
