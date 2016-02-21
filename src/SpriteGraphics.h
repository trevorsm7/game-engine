#ifndef __SPRITEGRAPHICS_H__
#define __SPRITEGRAPHICS_H__

#include "IGraphics.h"

#include <string>

class SpriteGraphics : public IGraphics
{
    Actor* m_actor;
    std::string m_filename;
    struct {float r, g, b;} m_color;
    struct {float x, y;} m_repeat;
    bool m_visible;

public:
    SpriteGraphics(Actor* actor): m_actor(actor), m_color{1.f, 1.f, 1.f}, m_repeat{1.f, 1.f}, m_visible(true) {}
    ~SpriteGraphics() override {}

    void setFilename(const char* filename) {m_filename = filename;}
    void setRepeat(float x, float y) {m_repeat = {x, y};}

    void update(float delta) override {}

    void render(IRenderer* renderer) override
    {
        if (!m_visible)
            return;

        renderer->setColor(m_color.r, m_color.g, m_color.b);
        renderer->drawSprite(m_filename, 0.f, 0.f, m_repeat.x, m_repeat.y);
    }

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

    void setColor(float r, float g, float b) override {m_color = {r, g, b};}
    void setVisible(bool visible) override {m_visible = visible;}
    bool isVisible() const override {return m_visible;}
};

#endif
