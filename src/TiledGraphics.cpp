#include "TiledGraphics.h"
#include "TileMap.h"
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
    luaL_argcheck(L, (lua_rawget(L, 1) == LUA_TUSERDATA), 1, "tilemap userdata required");
    m_tilemap = TileMap::checkUserdata(L, -1);
    m_tilemap->refAdded(L, -1);
    lua_pop(L, 1);
}

void TiledGraphics::destroy(lua_State* L)
{
    if (m_tilemap)
    {
        m_tilemap->refRemoved(L);
        m_tilemap = nullptr;
    }
}

// NOTE constexpr declaration requires a definition
const luaL_Reg TiledGraphics::METHODS[];

int TiledGraphics::script_getTileMap(lua_State* L)
{
    // Validate function arguments
    TiledGraphics* graphics = TUserdata<TiledGraphics>::checkUserdata(L, 1);

    // TODO if we allow a null tilemap later, remove the assert and return 0
    TileMap* tilemap = graphics->m_tilemap;
    assert(tilemap != nullptr);
    tilemap->pushUserdata(L);

    return 1;
}
