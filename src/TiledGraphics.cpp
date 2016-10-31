#include "TiledGraphics.hpp"
#include "Serializer.hpp"
#include "IRenderer.hpp"
#include "TileMap.hpp"
#include "Actor.hpp"

const luaL_Reg TiledGraphics::METHODS[];

void TiledGraphics::render(IRenderer* renderer)
{
    if (!isVisible() || !m_tilemap)
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
    lua_pushliteral(L, "tilemap");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setChild(L, m_tilemap, -1);
    lua_pop(L, 1);
}

void TiledGraphics::clone(lua_State* L, TiledGraphics* source)
{
    if (source->m_tilemap)
    {
        // Don't need to clone TileMap; just copy
        //source->m_tilemap->pushClone(L);
        source->m_tilemap->pushUserdata(L);
        setChild(L, m_tilemap, -1);
        lua_pop(L, 1);
    }
}

void TiledGraphics::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    if (m_tilemap)
    {
        m_tilemap->pushUserdata(L);
        serializer->serializeMember(ref, "", "tilemap", "setTilemap", L, -1);
        lua_pop(L, 1);
    }
}

int TiledGraphics::script_getTileMap(lua_State* L)
{
    TiledGraphics* graphics = TiledGraphics::checkUserdata(L, 1);
    return pushMember(L, graphics->m_tilemap);
}

int TiledGraphics::script_setTileMap(lua_State* L)
{
    TiledGraphics* graphics = TiledGraphics::checkUserdata(L, 1);
    graphics->setChild(L, graphics->m_tilemap, 2);
    return 0;
}
