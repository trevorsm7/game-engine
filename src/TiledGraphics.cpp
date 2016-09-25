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

// TODO refactor with other similar functions
void TiledGraphics::setTileMap(lua_State* L, int index)
{
    TileMap* tilemap = TileMap::checkUserdata(L, index);

    // Do nothing if we already own the component
    if (m_tilemap == tilemap)
        return;

    // Clear old component first
    if (m_tilemap != nullptr)
        m_tilemap->refRemoved(L);

    // Add component to new actor
    tilemap->refAdded(L, index);
    m_tilemap = tilemap;
}

void TiledGraphics::construct(lua_State* L)
{
    lua_pushliteral(L, "tilemap");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setTileMap(L, -1);
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
    // Validate function arguments
    TiledGraphics* graphics = TiledGraphics::checkUserdata(L, 1);

    TileMap* tilemap = graphics->m_tilemap;
    if (!tilemap)
        return 0;

    tilemap->pushUserdata(L);
    return 1;
}

int TiledGraphics::script_setTileMap(lua_State* L)
{
    // Validate function arguments
    TiledGraphics* graphics = TiledGraphics::checkUserdata(L, 1);

    graphics->setTileMap(L, 2);

    return 0;
}
