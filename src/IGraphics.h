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

    virtual void setColor(float r, float g, float b) = 0;
};

#endif
