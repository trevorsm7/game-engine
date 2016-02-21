#ifndef __BASICCAMERA_H__
#define __BASICCAMERA_H__

#include "ICamera.h"
#include "Transform.h"

class BasicCamera : public ICamera
{
    Transform m_transform;
    float m_width, m_height;
    struct {float x, y;} m_center;
    bool m_fixed;

public:
    BasicCamera(float w, float h, bool fixed): m_transform(0.f, 0.f, w, h),
        m_width(w), m_height(h), m_center{w/2, h/2}, m_fixed(fixed) {}
    ~BasicCamera() override {}

    void resize(int width, int height) override;

    void preRender(IRenderer* renderer) override;
    void postRender(IRenderer* renderer) override;

    void setCenter(float x, float y) override;

    float cameraToWorldX(int x, int l, int r) override;
    float cameraToWorldY(int y, int b, int t) override;
};

#endif
