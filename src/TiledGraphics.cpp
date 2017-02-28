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

bool TiledGraphics::testBounds(float x, float y, float& xl, float& yl) const
{
    assert(m_actor != nullptr);

    if (!isVisible() || !m_tilemap)
        return false;

    Aabb bounds = m_actor->getAabb();
    if (!bounds.isContaining(x, y))
        return false;

    const float width = bounds.getWidth();
    const float height = bounds.getHeight();

    xl = (x - bounds.getLeft()) * m_tilemap->getCols() / width;
    yl = (y - bounds.getTop()) * m_tilemap->getRows() / height;
    //int index = m_tilemap->getIndex(int(xl), int(yl));
    //return (index != 0);
    return true; // Actor can decide whether or not to cull
}

void TiledGraphics::getSize(float& w, float& h) const
{
    if (m_tilemap)
    {
        w = float(m_tilemap->getCols());
        h = float(m_tilemap->getRows());
    }
    else
    {
        IGraphics::getSize(w, h);
    }
}

void TiledGraphics::construct(lua_State* L)
{
    getChildOpt(L, 2, "tilemap", m_tilemap);
}

void TiledGraphics::clone(lua_State* L, TiledGraphics* source)
{
    copyChild(L, m_tilemap, source->m_tilemap);
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
    graphics->setChild(L, 2, graphics->m_tilemap);
    return 0;
}
