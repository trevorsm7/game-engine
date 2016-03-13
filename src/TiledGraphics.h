#ifndef __TILEDGRAPHICS_H__
#define __TILEDGRAPHICS_H__

#include "IGraphics.h"
#include "Actor.h"

#include <string>
#include <vector>

class TiledGraphics : public IGraphics
{
    Actor* m_actor;
    std::string m_tilemap;
    //std::vector<struct {char r, g, b;}> m_colors;
    // matching collision mask in TiledCollider??
    bool m_visible;

public:
    TiledGraphics(Actor* actor, std::string tilemap): m_actor(actor), m_tilemap(tilemap), m_visible(true) {}
    ~TiledGraphics() override {}

    void update(float delta) override {}

    void render(IRenderer* renderer) override
    {
        if (!m_visible)
            return;

        renderer->setColor(1.f, 1.f, 1.f);
        renderer->drawTiles(m_tilemap);
    }

    // TODO: should this behavior be different from SpriteGraphics?
    bool testBounds(float x, float y) const override
    {
        // TODO: shouldn't be able to click invisible sprite...
        // but we should make an IGraphics impl for invisible triggers
        //if (!m_visible)
        //    return false;

        Transform& transform = m_actor->getTransform();
        const float left = transform.getX();
        const float bottom = transform.getY();
        const float right = left + transform.getW();
        const float top = bottom + transform.getH();
        return (x >= left && x < right && y >= bottom && y < top);
    }

    void setColor(float r, float g, float b) override {/*TODO: no-op?*/}
    void setVisible(bool visible) override {m_visible = visible;}
    bool isVisible() const override {return m_visible;}
};

#endif
