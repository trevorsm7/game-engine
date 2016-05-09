#include "TiledGraphics.h"
#include "IRenderer.h"
#include "Actor.h"

void TiledGraphics::render(IRenderer* renderer)
{
    if (!isVisible())
        return;

    renderer->setColor(m_color.r, m_color.g, m_color.b);
    renderer->drawTiles(m_tilemap);
}

bool TiledGraphics::testBounds(float x, float y) const
{
    assert(m_actor != nullptr);

    // TODO: shouldn't be able to click invisible sprite...
    // but we should make an IGraphics impl for invisible triggers
    //if (!m_visible)
    //    return false;

    const Transform& transform = m_actor->getTransform();
    const float left = transform.getX();
    const float bottom = transform.getY();
    const float right = left + transform.getW();
    const float top = bottom + transform.getH();
    return (x >= left && x < right && y >= bottom && y < top);
}

void TiledGraphics::construct(lua_State* L)
{
    TGraphics<TiledGraphics>::construct(L);

    lua_pushliteral(L, "tilemap");
    luaL_argcheck(L, (lua_rawget(L, 1) == LUA_TSTRING), 1, "{tilemap = filename} is required");
    m_tilemap = lua_tostring(L, -1);
    lua_pop(L, 1);
}
