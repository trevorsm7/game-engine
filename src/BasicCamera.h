#ifndef __BASICCAMERA_H__
#define __BASICCAMERA_H__

#include "ICamera.h"
#include "Transform.h"

class BasicCamera : public ICamera
{
    Transform m_transform;

public:
    BasicCamera(): m_transform(0.f, 0.f, 20.f, 15.f) {}
    virtual ~BasicCamera() {}

    void update(float delta) override {}
    void preRender(IRenderer* renderer) override;
    void postRender(IRenderer* renderer) override;

    float cameraToWorldX(int x, int l, int r) override;
    float cameraToWorldY(int y, int b, int t) override;
};

#endif
