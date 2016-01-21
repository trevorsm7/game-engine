#ifndef __GRAPHICSCOMPONENT_H__
#define __GRAPHICSCOMPONENT_H__

//#include "Actor.h"
#include "Component.h"
#include "Renderer.h"

class GraphicsComponent: public Component
{
    struct {float r, g, b;} m_color;

public:
    GraphicsComponent(): m_color({1.f, 1.f, 1.f}) {}
    GraphicsComponent(float red, float green, float blue): m_color({red, green, blue}) {}
    ~GraphicsComponent() override {}

    void update(lua_State *state, float delta) override {}

    void setColor(float red, float green, float blue) {m_color = {red, green, blue};}

    //void setVisible(bool isVisible) {m_isVisible = isVisible;}
    virtual void render(IRenderer *renderer)
    {
        if (renderer)
        {
            renderer->setColor(m_color.r, m_color.g, m_color.b);
            renderer->drawSprite();
        }
    }
};

#endif
