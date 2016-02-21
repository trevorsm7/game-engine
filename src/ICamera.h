#ifndef __ICAMERA_H__
#define __ICAMERA_H__

#include "Transform.h"
#include "IRenderer.h"

class ICamera
{
public:
    virtual ~ICamera() {}

    virtual void resize(int width, int height) = 0;

    virtual void preRender(IRenderer* renderer) = 0;
    virtual void postRender(IRenderer* renderer) = 0;

    virtual void setCenter(float x, float y) = 0;

    virtual float cameraToWorldX(int x, int l, int r) const = 0;
    virtual float cameraToWorldY(int y, int b, int t) const = 0;
};

#endif
