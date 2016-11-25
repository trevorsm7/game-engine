#include "TiledCollider.hpp"
#include "Serializer.hpp"
#include "Actor.hpp"
#include "TileMap.hpp"

#include <cmath>
#include <algorithm>
#include <cassert>

const luaL_Reg TiledCollider::METHODS[];

bool TiledCollider::testCollision(float x, float y) const
{
    assert(m_actor != nullptr);

    if (!m_tilemap->getTileIndex())
        return false;

    if (!isCollidable() || !m_tilemap)
        return false;

    // Reject if outside of tilemap bounds
    Aabb bounds = m_actor->getAabb();
    if (!bounds.isContaining(x, y))
        return false;

    // Map to tile map coordinates
    // NOTE don't need floor() since x, y guranteed to be non-negative
    const int tileX = int((x - bounds.getLeft()) * m_tilemap->getCols() / bounds.getWidth());
    const int tileY = int((y - bounds.getTop()) * m_tilemap->getRows() / bounds.getHeight());
    if (!m_tilemap->isValidIndex(tileX, tileY))
        return false;

    return m_tilemap->isFlagSet(tileX, tileY, TileIndex::MoveBlocking);
}

bool TiledCollider::testCollision(const Aabb& aabb) const
{
    assert(m_actor != nullptr);

    if (!m_tilemap->getTileIndex())
        return false;

    if (!isCollidable() || !m_tilemap)
        return false;

    // Reject early if no overlap
    Aabb bounds = m_actor->getAabb();
    if (!bounds.isOverlapping(aabb))
        return false;

    // Translate AABB relative to transform
    const float left = aabb.getLeft() - bounds.getLeft();
    const float top = aabb.getTop() - bounds.getTop();
    const float right = aabb.getRight() - bounds.getLeft();
    const float bottom = aabb.getBottom() - bounds.getTop();

    // Map to tile map coordinates
    // NOTE using ceil for right/bottom since these are exclusive ranges
    const int tileLeft = std::max<int>(0, floor(left * m_tilemap->getCols() / bounds.getWidth()));
    const int tileRight = std::min<int>(m_tilemap->getCols(), ceil(right * m_tilemap->getCols() / bounds.getWidth()));
    const int tileTop = std::max<int>(0, floor(top * m_tilemap->getRows() / bounds.getHeight()));
    const int tileBottom = std::min<int>(m_tilemap->getRows(), ceil(bottom * m_tilemap->getRows() / bounds.getHeight()));

    // Iterate over range of tiles that overlap
    for (int y = tileTop; y < tileBottom; ++y)
    {
        for (int x = tileLeft; x < tileRight; ++x)
        {
            if (m_tilemap->isFlagSet(x, y, TileIndex::MoveBlocking))
                return true;
        }
    }

    return false;
}

// NOTE: testing a tilemap against an aabb will be much less effient than vise versa;
// this would probably only be reasonable when testing a tilemap against a tilemap
bool TiledCollider::testCollision(float deltaX, float deltaY, const ICollider* other) const
{
    assert(m_actor != nullptr);
    assert(other != nullptr);

    if (!m_tilemap->getTileIndex())
        return false;

    if (!isCollidableWith(other) || !m_tilemap)
        return false;

    // Reject early if there is no overlap with the map bounds
    Aabb bounds = m_actor->getAabb();
    bounds.addOffset(deltaX, deltaY);
    if (!other->testCollision(bounds))
        return false;

    // TODO might be more efficient to keep subdividing like a BVH
    // TODO another approach would be to combine adjacent collidable tiles into larger AABBs
    // NOTE could also add a getAabb to ICollider to use to restrict our range

    // Iterate over all tiles
    const int rows = m_tilemap->getRows();
    const int cols = m_tilemap->getCols();
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            if (!m_tilemap->isFlagSet(x, y, TileIndex::MoveBlocking))
                continue;

            // If collidable, compute AABB for tile
            const float left = bounds.getLeft() + (x * bounds.getWidth() / cols);
            const float right = bounds.getLeft() + ((x + 1) * bounds.getWidth() / cols);
            const float top = bounds.getTop() + (y * bounds.getHeight() / rows);
            const float bottom = bounds.getTop() + ((y + 1) * bounds.getHeight() / rows);
            const Aabb tileAabb(left, top, right, bottom);

            // Test the tile AABB against the collider
            if (other->testCollision(tileAabb))
                return true;
        }
    }

    return false;
}

bool TiledCollider::getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const
{
    // TODO need to implement
    return false;
}

bool TiledCollider::getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const
{
    // TODO need to implement
    return false;
}

void TiledCollider::construct(lua_State* L)
{
    lua_pushliteral(L, "tilemap");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setChild(L, m_tilemap, -1);
    lua_pop(L, 1);
}

void TiledCollider::clone(lua_State* L, TiledCollider* source)
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

void TiledCollider::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    serializer->serializeMember(ref, "", "tilemap", "setTilemap", L, m_tilemap);
}

int TiledCollider::script_getTileMap(lua_State* L)
{
    TiledCollider* collider = TiledCollider::checkUserdata(L, 1);
    return pushMember(L, collider->m_tilemap);
}

int TiledCollider::script_setTileMap(lua_State* L)
{
    TiledCollider* collider = TiledCollider::checkUserdata(L, 1);
    collider->setChild(L, collider->m_tilemap, 2);
    return 0;
}
