#pragma once

#include "IUserdata.hpp"
#include "Event.hpp"

#include <vector>
#include <memory>
#include "lua.hpp"

class Scene;
class Actor;
class ICamera;
class IRenderer;
class ResourceManager;

class Canvas : public TUserdata<Canvas>
{
    friend class Scene;

    typedef std::vector<Actor*> ActorVector;
    typedef ActorVector::const_iterator ActorIterator;

    ActorVector m_actors;
    ActorVector m_added;
    ICamera* m_camera = nullptr;
    Scene* m_scene = nullptr;
    bool m_paused = false;
    bool m_visible = true;
    bool m_actorRemoved = false;

    Canvas() = default;

public:
    ~Canvas() {}

    ResourceManager* getResourceManager() const;

    void update(lua_State* L, float delta);
    void render(IRenderer* renderer);
    bool mouseEvent(lua_State* L, MouseEvent& event);
    void resize(lua_State* L, int width, int height);

private:
    void processAddedActors(lua_State *L);
    void processRemovedActors(lua_State *L);
    void updatePhysics(lua_State *L, float delta);

public:
    bool testCollision(float deltaX, float deltaY, const Actor* actor1) const;
    bool getEarliestCollision(const Actor* actor1, ActorIterator it, ActorIterator itEnd, Actor*& hit, float& start, float& end, float& normX, float& normY) const;
    bool getEarliestCollision(const Actor* actor1, Actor*& hit, float& start, float& end, float& normX, float& normY) const
        {return getEarliestCollision(actor1, m_actors.begin(), m_actors.end(), hit, start, end, normX, normY);}

private:
    template <class T> void set(lua_State* L, T*& component, int index);
    template <class T> void remove(lua_State* L, T*& component);

private:
    friend class TUserdata<Canvas>;
    void construct(lua_State* L);
    void clone(lua_State* L, Canvas* source);
    void destroy(lua_State* L);
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int canvas_addActor(lua_State* L);
    static int canvas_removeActor(lua_State* L);
    static int canvas_clear(lua_State* L);
    static int canvas_getCamera(lua_State* L);
    static int canvas_setCamera(lua_State* L);
    static int canvas_setCenter(lua_State* L);
    static int canvas_setOrigin(lua_State* L);
    static int canvas_getCollision(lua_State* L);
    static int canvas_setPaused(lua_State* L);
    static int canvas_setVisible(lua_State* L);

    static constexpr const char* const CLASS_NAME = "Canvas";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"addActor", canvas_addActor},
        {"removeActor", canvas_removeActor},
        {"clear", canvas_clear},
        {"setCenter", canvas_setCenter},
        {"setOrigin", canvas_setOrigin},
        {"getCollision", canvas_getCollision},
        {"setPaused", canvas_setPaused},
        {"setVisible", canvas_setVisible},
        {nullptr, nullptr}
    };
};

template <class T>
inline void Canvas::set(lua_State* L, T*& component, int index)
{
    T* ptr = T::checkInterface(L, index);

    // Do nothing if we already own the component
    if (component == ptr)
        return;

    // Clear old component first
    if (component != nullptr)
    {
        assert(component->m_canvas == this);
        component->m_canvas = nullptr;
        releaseChild(L, component);
    }

    // If component already owned, clone it
    if (ptr->m_canvas != nullptr)
    {
        ptr->pushClone(L);
        component = T::testInterface(L, -1);
        assert(component != nullptr);
        acquireChild(L, component, -1);
        component->m_canvas = this;
        lua_pop(L, 1);
    }
    else
    {
        component = ptr;
        acquireChild(L, component, index);
        component->m_canvas = this;
    }
}

template <class T>
inline void Canvas::remove(lua_State* L, T*& component)
{
    if (component != nullptr)
    {
        assert(component->m_canvas == this);
        component->m_canvas = nullptr;
        releaseChild(L, component);
        component = nullptr;
    }
}
