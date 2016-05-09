#include "TiledCollider.h"
#include "Actor.h"
#include "TileMap.h"

#include <cmath>
#include <algorithm>
#include <cassert>

bool TiledCollider::testCollision(float x, float y) const
{
    assert(m_actor != nullptr);

    if (!isCollidable())
        return false;

    // Reject if outside of tilemap bounds
    const Transform& transform = m_actor->getTransform();
    x -= transform.getX();
    y -= transform.getY();
    if (x < 0.f || x >= transform.getW() || y < 0.f || y >= transform.getH())
        return false;

    // Get the resource manager
    ResourceManager* resources = m_actor->getResourceManager();
    if (!resources)
    {
        fprintf(stderr, "ResourceManager null!\n");
        return false;
    }

    // Load the tile map
    TileMapPtr tileMap = TileMap::loadTileMap(*resources, m_tilemap);
    if (!tileMap)
        return false;

    // Load the tile index
    TileIndexPtr tileIndex = TileIndex::loadTileIndex(*resources, tileMap->getIndexFile());
    if (!tileIndex)
        return false;

    // Map to tile map coordinates; y is inverted
    // NOTE don't need floor() since x, y guranteed to be non-negative
    const int tileX = int(x * tileMap->getCols() / transform.getW());
    //const int tileY = int((transform.getH() - y) * tileMap->getRows() / transform.getH());
    const int tileY = (tileMap->getRows() - 1) - int(y * tileMap->getRows() / transform.getH());

    // Get the collision flag at the tile map index
    int index = tileMap->getIndex(tileX, tileY);
    return tileIndex->isCollidable(index);
}

bool TiledCollider::testCollision(const Aabb& aabb) const
{
    assert(m_actor != nullptr);

    if (!isCollidable())
        return false;

    // Translate AABB relative to transform; reject if no overlap
    const Transform& transform = m_actor->getTransform();
    const float left = aabb.getLeft() - transform.getX();
    const float right = aabb.getRight() - transform.getX();
    const float bottom = aabb.getBottom() - transform.getY();
    const float top = aabb.getTop() - transform.getY();
    if (right <= 0.f || top <= 0.f || left >= transform.getW() || bottom >= transform.getH())
        return false;

    // Get the resource manager
    ResourceManager* resources = m_actor->getResourceManager();
    if (!resources)
    {
        fprintf(stderr, "ResourceManager null!\n");
        return false;
    }

    // Load the tile map
    TileMapPtr tileMap = TileMap::loadTileMap(*resources, m_tilemap);
    if (!tileMap)
        return false;

    // Load the tile index
    TileIndexPtr tileIndex = TileIndex::loadTileIndex(*resources, tileMap->getIndexFile());
    if (!tileIndex)
        return false;

    // Map to tile map coordinates; y is inverted
    // NOTE using ceil for right/top since these are exclusive ranges
    const int tileLeft = std::max<int>(0, floor(left * tileMap->getCols() / transform.getW()));
    const int tileRight = std::min<int>(tileMap->getCols(), ceil(right * tileMap->getCols() / transform.getW()));
    const int tileBottom = tileMap->getRows() - std::max<int>(0, floor(bottom * tileMap->getRows() / transform.getH()));
    const int tileTop = tileMap->getRows() - std::min<int>(tileMap->getRows(), ceil(top * tileMap->getRows() / transform.getH()));

    // Iterate over range of tiles that overlap
    for (int y = tileTop; y < tileBottom; ++y)
    {
        for (int x = tileLeft; x < tileRight; ++x)
        {
            // Get the collision flag at the tile map index
            int index = tileMap->getIndex(x, y);
            if (tileIndex->isCollidable(index))
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

    if (!isCollidableWith(other))
        return false;

    // Reject early if there is no overlap with the map bounds
    const Transform& transform = m_actor->getTransform();
    if (!other->testCollision(transform.getAabb()))
        return false;

    // Get the resource manager
    ResourceManager* resources = m_actor->getResourceManager();
    if (!resources)
    {
        fprintf(stderr, "ResourceManager null!\n");
        return false;
    }

    // Load the tile map
    TileMapPtr tileMap = TileMap::loadTileMap(*resources, m_tilemap);
    if (!tileMap)
        return false;

    // Load the tile index
    TileIndexPtr tileIndex = TileIndex::loadTileIndex(*resources, tileMap->getIndexFile());
    if (!tileIndex)
        return false;

    // TODO might be more efficient to keep subdividing like a BVH
    // TODO another approach would be to combine adjacent collidable tiles into larger AABBs
    // NOTE could also add a getAabb to ICollider to use to restrict our range

    // Iterate over all tiles
    const int rows = tileMap->getRows();
    const int cols = tileMap->getCols();
    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < cols; ++x)
        {
            // Get the collision flag at the tile map index
            int index = tileMap->getIndex(x, y);
            if (!tileIndex->isCollidable(index))
                continue;

            // If collidable, compute AABB for tile; y is inverted
            const float left = transform.getX() + (x * transform.getW() / cols) + deltaX;
            const float right = transform.getX() + ((x + 1) * transform.getW() / cols) + deltaX;
            const float top = transform.getY() + ((rows - y) * transform.getH() / rows) + deltaY;
            const float bottom = transform.getY() + ((rows - y - 1) * transform.getH() / rows) + deltaY;
            const Aabb tileAabb(left, bottom, right, top);

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
    TCollider<TiledCollider>::construct(L);

    lua_pushliteral(L, "tilemap");
    luaL_argcheck(L, (lua_rawget(L, 1) == LUA_TSTRING), 1, "{tilemap = filename} is required");
    m_tilemap = lua_tostring(L, -1);
    lua_pop(L, 1);
}
