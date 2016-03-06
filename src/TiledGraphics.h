#ifndef __TILEDGRAPHICS_H__
#define __TILEDGRAPHICS_H__

#include "IGraphics.h"
#include "Actor.h"

#include <string>
#include <vector>

class TiledGraphics : public IGraphics
{
    Actor* m_actor;
    TileIndex m_index;
    TileMap m_tiles;
    //std::vector<struct {char r, g, b;}> m_colors;
    // matching collision mask in TiledCollider??
    bool m_visible;

public:
    TiledGraphics(Actor* actor): m_actor(actor), m_visible(true) {}
    ~TiledGraphics() override {}

    // TODO: these are gross
    void setIndex(const TileIndex& index) {m_index = index;}
    void setTiles(const TileMap& tiles) {m_tiles = tiles;}

    void update(float delta) override {}

    void render(IRenderer* renderer) override
    {
        if (!m_visible)
            return;

        renderer->setColor(1.f, 1.f, 1.f);
        renderer->drawTiles(m_index, m_tiles);
    }

    // TODO: should this behavior be different from SpriteGraphics?
    bool testBounds(float x, float y) const override
    {
        // TODO: shouldn't be able to click invisible sprite...
        // but we should make an IGraphics impl for invisible triggers
        //if (!m_visible)
        //    return false;

        Transform& transform = m_actor->getTransform();
        float left = transform.getX();
        float bottom = transform.getY();
        float right = left + transform.getW();
        float top = bottom + transform.getH();
        return (x >= left && x < right && y >= bottom && y < top);
    }

    void setColor(float r, float g, float b) override {/*TODO: no-op?*/}
    void setVisible(bool visible) override {m_visible = visible;}
    bool isVisible() const override {return m_visible;}
};

#endif
