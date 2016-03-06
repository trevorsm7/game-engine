#ifndef __ICAMERA_H__
#define __ICAMERA_H__

#include "Transform.h"
#include "IRenderer.h"
#include "Event.h"

class ICamera
{
public:
    virtual ~ICamera() {}

    virtual void resize(int width, int height) = 0;

    virtual void preRender(IRenderer* renderer) = 0;
    virtual void postRender(IRenderer* renderer) = 0;

    virtual void setCenter(float x, float y) = 0;

    virtual void mouseToWorld(const MouseEvent& event, float& x, float& y) const = 0;
};

#endif
