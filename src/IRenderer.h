#ifndef __IRENDERER_H__
#define __IRENDERER_H__

#include "Transform.h"

#include <string>

class IRenderer
{
public:
    virtual ~IRenderer() {}

    virtual void init() = 0;
    virtual void preRender() = 0;
    virtual void postRender() = 0;

    virtual void setViewport(int left, int bottom, int right, int top) = 0;
    virtual void pushModelTransform(Transform& transform) = 0;
    virtual void pushCameraTransform(Transform& transform) = 0;

    virtual void setColor(float red, float green, float blue) = 0;
    virtual void drawSprite(const std::string& name, float l, float b, float w, float h) = 0;

    virtual void popModelTransform() = 0;
    virtual void popCameraTransform() = 0;
};

#endif
