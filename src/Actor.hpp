#pragma once

#include "IUserdata.hpp"
#include "Physics.hpp"
#include "Transform.hpp"

#include <memory>
#include <cassert>

struct lua_State;

class Canvas;
class IGraphics;
class ICollider;
class IPathing;
class IRenderer;
class ResourceManager;

class Actor : public TUserdata<Actor>
{
    // TODO either make into a component or add directly to class
    typedef std::unique_ptr<Physics> PhysicsPtr;

public:
    Canvas* m_canvas;

private:
    Transform m_transform;
    PhysicsPtr m_physics;
    IGraphics* m_graphics;
    ICollider* m_collider;
    IPathing* m_pathing;
    int m_layer; // TODO: probably want to move this to graphics later

    Actor(): m_canvas(nullptr), m_graphics(nullptr), m_collider(nullptr), m_pathing(nullptr), m_layer(0) {}

public:
    ~Actor() {}

    ResourceManager* getResourceManager() const;
    Transform& getTransform() {return m_transform;}
    Physics* getPhysics() {return m_physics.get();}
    const Physics* getPhysics() const {return m_physics.get();}
    const IGraphics* getGraphics() const {return m_graphics;}
    const ICollider* getCollider() const {return m_collider;}
    const IPathing* getPathing() const {return m_pathing;}

    void setLayer(int layer) {m_layer = layer;}
    int getLayer() const {return m_layer;}

    void update(lua_State* L, float delta);
    void render(IRenderer* renderer);
    bool mouseEvent(lua_State* L, bool down);
    void collideEvent(lua_State* L, Actor* with);

    bool testMouse(float x, float y) const;
    bool testCollision(float x, float y) const;

private:
    template <class T> void set(lua_State* L, T*& component, int index);
    template <class T> int push(lua_State* L, T*& component);
    template <class T> void remove(lua_State* L, T*& component);

private:
    friend class TUserdata<Actor>;
    void construct(lua_State* L);
    void clone(lua_State* L, Actor* source);
    void destroy(lua_State* L);
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int actor_getCanvas(lua_State* L);
    static int actor_getGraphics(lua_State* L);
    static int actor_setGraphics(lua_State* L);
    static int actor_getCollider(lua_State* L);
    static int actor_setCollider(lua_State* L);
    static int actor_getPathing(lua_State* L);
    static int actor_setPathing(lua_State* L);
    static int actor_getPosition(lua_State* L);
    static int actor_setPosition(lua_State* L);
    static int actor_setScale(lua_State* L);
    static int actor_testCollision(lua_State* L);
    static int actor_setVelocity(lua_State* L);
    static int actor_getVelocity(lua_State* L);
    static int actor_addAcceleration(lua_State* L);

    static constexpr const char* const CLASS_NAME = "Actor";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"getCanvas", actor_getCanvas},
        {"getGraphics", actor_getGraphics},
        {"setGraphics", actor_setGraphics},
        {"getCollider", actor_getCollider},
        {"setCollider", actor_setCollider},
        {"getPathing", actor_getPathing},
        {"setPathing", actor_setPathing},
        {"getPosition", actor_getPosition},
        {"setPosition", actor_setPosition},
        {"setScale", actor_setScale},
        {"testCollision", actor_testCollision},
        {"setVelocity", actor_setVelocity},
        {"getVelocity", actor_getVelocity},
        {"addAcceleration", actor_addAcceleration},
        {nullptr, nullptr}
    };
};

template <class T>
inline void Actor::set(lua_State* L, T*& component, int index)
{
    T* ptr = T::checkInterface(L, index);

    // Do nothing if we already own the component
    if (component == ptr)
        return;

    // Clear old component first
    if (component != nullptr)
    {
        assert(component->m_actor == this);
        component->m_actor = nullptr;
        component->refRemoved(L);
        //own = nullptr;
    }

    // If component already owned, clone it
    if (ptr->m_actor != nullptr)
    {
        ptr->pushClone(L);
        component = T::testInterface(L, -1);
        assert(component != nullptr);
        component->refAdded(L, -1);
        component->m_actor = this;
        lua_pop(L, 1);
    }
    else
    {
        component = ptr;
        component->refAdded(L, index);
        component->m_actor = this;
    }
}

template <class T>
inline int Actor::push(lua_State* L, T*& component)
{
    if (component != nullptr)
    {
        component->pushUserdata(L);
        return 1;
    }

    return 0;
}

template <class T>
inline void Actor::remove(lua_State* L, T*& component)
{
    if (component != nullptr)
    {
        component->m_actor = nullptr;
        component->refRemoved(L);
        component = nullptr;
    }
}
