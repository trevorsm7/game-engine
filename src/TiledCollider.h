#ifndef __TILEDCOLLIDER_H__
#define __TILEDCOLLIDER_H__

#include "ICollider.h"
#include "Actor.h"
#include "TileMap.h"

class TiledCollider : public ICollider
{
    Actor* m_actor;
    std::string m_tilemap;
    bool m_collidable;

public:
    TiledCollider(Actor* actor, std::string tilemap): m_actor(actor), m_tilemap(tilemap), m_collidable(true) {}
    ~TiledCollider() override {}

    void update(float delta) override {}

    bool testCollision(float x, float y) const override
    {
        if (!m_collidable)
            return false;

        if (!m_actor)
            return false;

        // Reject if outside of tilemap bounds
        Transform& transform = m_actor->getTransform();
        x -= transform.getX();
        y -= transform.getY();
        if (x < 0 || x >= transform.getW() || y < 0 || y >= transform.getH())
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
        const int tileX = int(x * tileMap->getCols() / transform.getW());
        //const int tileY = int((transform.getH() - y) * tileMap->getRows() / transform.getH());
        const int tileY = tileMap->getRows() - int(y * tileMap->getRows() / transform.getH()) - 1;

        // Get the collision flag at the tile map index
        int index = tileMap->getIndex(tileX, tileY);
        return tileIndex->getCollidable(index);
    }

    void setCollidable(bool collidable) override {m_collidable = collidable;}
};

#endif
