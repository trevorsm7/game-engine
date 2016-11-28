#pragma once

#include "IUserdata.hpp"

class Actor;
class IRenderer;

class IGraphics : public TUserdata<IGraphics>
{
public:
    Actor* m_actor; // TODO can make private?

protected:
    struct {float r, g, b;} m_color;
    bool m_isVisible;

    IGraphics(): m_actor(nullptr), m_color{1.f, 1.f, 1.f}, m_isVisible(true) {}

public:
    virtual ~IGraphics() {}

    virtual void update(float delta) = 0;
    virtual void render(IRenderer* renderer) = 0;

    // TODO: test click/ray for mouse events
    virtual bool testBounds(float x, float y, float& xl, float& yl) const;

    virtual void getSize(float& w, float& h) const {w = 1.f; h = 1.f;}

    bool isVisible() const {return m_isVisible;}
    void setVisible(bool visible) {m_isVisible = visible;}
    void setColor(float r, float g, float b) {m_color = {r, g, b};}

private:
    friend class TUserdata<IGraphics>;
    void construct(lua_State* L);
    void clone(lua_State* L, IGraphics* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_isVisible(lua_State* L);
    static int script_setVisible(lua_State* L);
    static int script_setColor(lua_State* L);

    static constexpr const char* const CLASS_NAME = "IGraphics";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"isVisible", script_isVisible},
        {"setVisible", script_setVisible},
        {"setColor", script_setColor},
        {nullptr, nullptr}
    };
};
