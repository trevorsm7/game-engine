#ifndef __SPRITEGRAPHICS_H__
#define __SPRITEGRAPHICS_H__

#include "IGraphics.h"

class SpriteGraphics : public IGraphics
{
    struct {float r, g, b;} m_color;

public:
    SpriteGraphics(): m_color({1.f, 1.f, 1.f}) {}
    SpriteGraphics(float red, float green, float blue): m_color({red, green, blue}) {}
    virtual ~SpriteGraphics() {}

    void update(float delta) override {}

    void render(IRenderer* renderer) override
    {
        renderer->setColor(m_color.r, m_color.g, m_color.b);
        renderer->drawSprite();
    }

    void setColor(float r, float g, float b) override {m_color = {r, g, b};}
};

#endif
