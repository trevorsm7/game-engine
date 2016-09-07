#pragma once

#include "IUserdata.hpp"
#include "Physics.hpp"
#include "Transform.hpp"

#include <memory>
#include "lua.hpp"

class Canvas;
class IGraphics;
class ICollider;
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
    int m_layer; // TODO: probably want to move this to graphics later

    Actor(): m_canvas(nullptr), m_graphics(nullptr), m_collider(nullptr), m_layer(0) {}

public:
    ~Actor() {}

    ResourceManager* getResourceManager() const;
    Transform& getTransform() {return m_transform;}
    Physics* getPhysics() {return m_physics.get();}
    const Physics* getPhysics() const {return m_physics.get();}
    const IGraphics* getGraphics() const {return m_graphics;}
    const ICollider* getCollider() const {return m_collider;}

    void setLayer(int layer) {m_layer = layer;}
    int getLayer() const {return m_layer;}

    void update(lua_State* L, float delta);
    void render(IRenderer* renderer);
    bool mouseEvent(lua_State* L, bool down);
    void collideEvent(lua_State* L, Actor* with);

    bool testMouse(float x, float y) const;
    bool testCollision(float x, float y) const;

private:
    void setGraphics(lua_State* L, int index);
    void setCollider(lua_State* L, int index);

private:
    friend class TUserdata<Actor>;
    void construct(lua_State* L);
    void destroy(lua_State* L);
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int actor_getCanvas(lua_State* L);
    static int actor_getGraphics(lua_State* L);
    static int actor_setGraphics(lua_State* L);
    static int actor_getCollider(lua_State* L);
    static int actor_setCollider(lua_State* L);
    static int actor_getPosition(lua_State* L);
    static int actor_setPosition(lua_State* L);
    static int actor_setScale(lua_State* L);
    static int actor_testCollision(lua_State* L);
    //static int actor_getEarliestCollision(lua_State* L);
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
        {"getPosition", actor_getPosition},
        {"setPosition", actor_setPosition},
        {"setScale", actor_setScale},
        {"testCollision", actor_testCollision},
        //{"getEarliestCollision", actor_getEarliestCollision},
        {"setVelocity", actor_setVelocity},
        {"getVelocity", actor_getVelocity},
        {"addAcceleration", actor_addAcceleration},
        {nullptr, nullptr}
    };
};
