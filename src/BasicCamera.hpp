#pragma once

#include "ICamera.hpp"
#include "Transform.hpp"

class BasicCamera : public ICamera
{
    Transform m_transform;
    float m_width, m_height;
    struct {float x, y;} m_center;
    bool m_fixed;

public:
    BasicCamera(): m_width(20.f), m_height(15.f), m_center{10.f, 7.5f}, m_fixed(true) {}
    ~BasicCamera() override {}

    void construct(lua_State* L, int index) override;
    void serialize(lua_State* L, const char* table, ObjectRef* ref) const override;

    void resize(int width, int height) override;

    void preRender(IRenderer* renderer) override;
    void postRender(IRenderer* renderer) override;

    void setCenter(float x, float y) override;
    void setOrigin(float x, float y) override;

    void mouseToWorld(const MouseEvent& event, float& x, float& y) const override;
};
