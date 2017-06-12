#pragma once

#include "IUserdata.hpp"
#include "Event.hpp"

class Canvas;
class IRenderer;

//struct lua_State;
//class Serializer;
//class ObjectRef;

class ICamera : public TUserdata<ICamera>
{
public:
    Canvas* m_canvas = nullptr;

protected:
    ICamera() = default;

public:
    virtual ~ICamera() {}

    virtual void resize(int width, int height) = 0;

    virtual void preRender(IRenderer* renderer) = 0;
    virtual void postRender(IRenderer* renderer) = 0;

    virtual void setCenter(float x, float y) = 0;
    virtual void setOrigin(float x, float y) = 0;

    virtual void mouseToWorld(const MouseEvent& event, float& x, float& y) const = 0;

private:
    friend class TUserdata<ICamera>;
    //void construct(lua_State* L);
    //void clone(lua_State* L, ICamera* source);
    //void destroy(lua_State* L) {}
    //void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static constexpr const char* const CLASS_NAME = "ICamera";
    static constexpr const luaL_Reg METHODS[] = {{nullptr, nullptr}};
};
