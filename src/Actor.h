#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "IRenderer.h"
#include "IGraphics.h"
#include "ICollider.h"
#include "Transform.h"
#include "Event.h"

#include <memory>
#include "lua.hpp"

class Canvas;

class Actor
{
    friend class Canvas;

    typedef std::unique_ptr<IGraphics> IGraphicsPtr;
    typedef std::unique_ptr<ICollider> IColliderPtr;

public:
    Transform m_transform; // TODO: how/who can access this???
private:
    IGraphicsPtr m_graphics;
    IColliderPtr m_collider;
    Canvas* m_canvas;
    int m_refCount;
    bool m_visible; // TODO: consider moving visibility into IGraphics

public:
    Actor(): m_canvas(nullptr), m_refCount(0), m_visible(true) {}
    ~Actor() {}

    void update(lua_State* L, float delta);
    void render(IRenderer* renderer);
    bool mouseEvent(lua_State* L, bool down);//MouseEvent& event);

    bool testMouse(float x, float y);
    bool testCollision(float x, float y);

//private:
    void refAdded(lua_State* L, int index);
    void refRemoved(lua_State* L);

    // TODO: may want to move scripting stuff to another file/class
    static int actor_init(lua_State* L);
    static int actor_create(lua_State* L);
    static int actor_delete(lua_State* L);
    static int actor_index(lua_State* L);
    static int actor_newindex(lua_State* L);
    static int actor_getPosition(lua_State* L);
    static int actor_setPosition(lua_State* L);
    static int actor_setScale(lua_State* L);
    static int actor_setColor(lua_State* L);
    static int actor_setVisible(lua_State* L);
    static int actor_isVisible(lua_State* L);
};

#endif
