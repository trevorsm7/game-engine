#pragma once

#include "ICamera.hpp"
#include "Transform.hpp"

struct lua_State;
class Serializer;
class ObjectRef;

class Camera2D : public TUserdata<Camera2D, ICamera>
{
    Transform m_transform;
    float m_width = 20.f, m_height = 15.f;
    struct {float x, y;} m_center = {10.f, 7.5f};
    bool m_fixed = false;

    Camera2D() = default;

public:
    ~Camera2D() override {}

    void resize(int width, int height) override;

    void preRender(IRenderer* renderer) override;
    void postRender(IRenderer* renderer) override;

    void setCenter(float x, float y) override;
    void setOrigin(float x, float y) override;

    void mouseToWorld(const MouseEvent& event, float& x, float& y) const override;

private:
    friend class TUserdata<Camera2D, ICamera>;
    void construct(lua_State* L);
    void clone(lua_State* L, Camera2D* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static constexpr const char* const CLASS_NAME = "Camera2D";
    static constexpr const luaL_Reg METHODS[] = {{nullptr, nullptr}};
};
