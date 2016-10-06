#pragma once

#include "Event.hpp"

class IRenderer;

struct lua_State;
class Serializer;
class ObjectRef;

class ICamera
{
public:
    virtual ~ICamera() {}

    virtual void construct(lua_State* L, int index) = 0;
    virtual void serialize(lua_State* L, const char* table, Serializer* serializer, ObjectRef* ref) const = 0;

    virtual void resize(int width, int height) = 0;

    virtual void preRender(IRenderer* renderer) = 0;
    virtual void postRender(IRenderer* renderer) = 0;

    virtual void setCenter(float x, float y) = 0;
    virtual void setOrigin(float x, float y) = 0;

    virtual void mouseToWorld(const MouseEvent& event, float& x, float& y) const = 0;
};
