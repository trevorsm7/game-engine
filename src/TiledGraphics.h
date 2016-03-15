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
    struct {float r, g, b;} m_color;
    //std::vector<struct {char r, g, b;}> m_colors;

public:
    TiledGraphics(Actor* actor, std::string tilemap): m_actor(actor), m_tilemap(tilemap), m_color{1.f, 1.f, 1.f} {}
    ~TiledGraphics() override {}

    void update(float delta) override {}

    void render(IRenderer* renderer) override
    {
        if (!isVisible())
            return;

        renderer->setColor(m_color.r, m_color.g, m_color.b);
        renderer->drawTiles(m_tilemap);
    }

    // TODO: should this behavior be different from SpriteGraphics?
    bool testBounds(float x, float y) const override
    {
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

    void setColor(float r, float g, float b) override {m_color = {r, g, b};}
};

#endif
