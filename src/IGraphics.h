#ifndef __IGRAPHICS_H__
#define __IGRAPHICS_H__

#include "IRenderer.h"

class IGraphics
{
public:
    //IGraphics() {}
    virtual ~IGraphics() {}

    virtual void update(float delta) = 0;
    virtual void render(IRenderer* renderer) = 0;

    // TODO: test click/ray for mouse events
    virtual bool testBounds(float x, float y) = 0;

    virtual void setColor(float r, float g, float b) = 0;
    virtual void setVisible(bool visible) = 0;
    virtual bool isVisible() = 0;
};

#endif
