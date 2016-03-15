#ifndef __IGRAPHICS_H__
#define __IGRAPHICS_H__

#include "IRenderer.h"

class IGraphics
{
    bool m_isVisible;

public:
    IGraphics(): m_isVisible(true) {}
    virtual ~IGraphics() {}

    virtual void update(float delta) = 0;
    virtual void render(IRenderer* renderer) = 0;

    // TODO: test click/ray for mouse events
    virtual bool testBounds(float x, float y) const = 0;

    // TODO provide default impl. and maybe remove virtual?
    virtual void setColor(float r, float g, float b) = 0;

    // TODO remove virtual if we don't need to override
    virtual bool isVisible() const {return m_isVisible;}
    virtual void setVisible(bool visible) {m_isVisible = visible;}
};

#endif
