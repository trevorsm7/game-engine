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
    renderer->drawLines(m_tilemap->m_debug); // TODO remove
}

bool TiledGraphics::testBounds(float x, float y) const
{
    assert(m_actor != nullptr);

    if (!isVisible() || !m_tilemap)
        return false;

    Aabb bounds = m_actor->getAabb();
    if (!bounds.isContaining(x, y))
        return false;

    const float width = bounds.getWidth();
    const float height = bounds.getHeight();

    // TODO check for invisible/masked tiles
    int tileX = (x - bounds.getLeft()) * m_tilemap->getCols() / width;
    int tileY = (y - bounds.getTop()) * m_tilemap->getRows() / height;
    int index = m_tilemap->getIndex(tileX, tileY);
    return (index != 0);
}

void TiledGraphics::getSize(float& w, float& h) const
{
    if (m_tilemap)
    {
        w = m_tilemap->getCols();
        h = m_tilemap->getRows();
    }
    else
    {
        IGraphics::getSize(w, h);
    }
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
        source->m_tilemap->pushUserdata(L); // shallow copy
        setChild(L, m_tilemap, -1);
        lua_pop(L, 1);
    }
}

void TiledGraphics::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    serializer->serializeMember(ref, "", "tilemap", "setTilemap", L, m_tilemap);
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
