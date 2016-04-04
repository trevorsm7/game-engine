#ifndef __ACTOR_H__
#define __ACTOR_H__

#include "IRenderer.h"
#include "IGraphics.h"
#include "ICollider.h"
#include "Physics.h"
#include "Transform.h"
#include "Event.h"
#include "ResourceManager.h"

#include <memory>
#include "lua.hpp"

class Canvas;

class Actor
{
    typedef std::unique_ptr<IGraphics> IGraphicsPtr;
    typedef std::unique_ptr<ICollider> IColliderPtr;
    typedef std::unique_ptr<Physics> PhysicsPtr;

public:
    Canvas* m_canvas;

private:
    Transform m_transform;
    PhysicsPtr m_physics;
    IGraphicsPtr m_graphics;
    IColliderPtr m_collider;
    int m_layer; // TODO: probably want to move this to graphics later
    int m_refCount;

    Actor(): m_canvas(nullptr), m_layer(0), m_refCount(0) {}

public:
    ~Actor() {}

    ResourceManager* getResourceManager() const;
    Transform& getTransform() {return m_transform;}
    Physics* getPhysics() {return m_physics.get();}
    const Physics* getPhysics() const {return m_physics.get();}
    const IGraphics* getGraphics() const {return m_graphics.get();}
    const ICollider* getCollider() const {return m_collider.get();}

    void setLayer(int layer) {m_layer = layer;}
    int getLayer() const {return m_layer;}

    void update(lua_State* L, float delta);
    void render(IRenderer* renderer);
    bool mouseEvent(lua_State* L, bool down);
    void collideEvent(lua_State* L, Actor* with);

    bool testMouse(float x, float y) const;
    bool testCollision(float x, float y) const;

    void refAdded(lua_State* L, int index);
    void refRemoved(lua_State* L);

private:
    bool pcall(lua_State* L, const char* method, int in, int out);

public:
    static int actor_init(lua_State* L);
    static constexpr const char* const METATABLE = "Actor";

private:
    static int actor_create(lua_State* L);
    static int actor_delete(lua_State* L);
    static int actor_index(lua_State* L);
    static int actor_newindex(lua_State* L);
    static int actor_getCanvas(lua_State* L);
    static int actor_getPosition(lua_State* L);
    static int actor_setPosition(lua_State* L);
    static int actor_setScale(lua_State* L);
    static int actor_setColor(lua_State* L);
    static int actor_setVisible(lua_State* L);
    static int actor_isVisible(lua_State* L);
    static int actor_setCollidable(lua_State* L);
    static int actor_testCollision(lua_State* L);
    //static int actor_getEarliestCollision(lua_State* L);
    static int actor_setVelocity(lua_State* L);
    static int actor_getVelocity(lua_State* L);
    static int actor_addAcceleration(lua_State* L);
};

#endif
