#pragma once

#include "IUserdata.hpp"
#include "Event.hpp"
#include "ICamera.hpp"

#include <vector>
#include <memory>
#include "lua.hpp"

class Scene;
class Actor;
class Pathfinding;
class ResourceManager;

class Canvas : public TUserdata<Canvas>
{
    friend class Scene;

    typedef std::unique_ptr<ICamera> ICameraPtr;
    typedef std::vector<Actor*> ActorVector;
    typedef ActorVector::const_iterator ActorIterator;

    ActorVector m_actors;
    ActorVector m_added;
    ICameraPtr m_camera;
    Pathfinding* m_pathfinding;
    Scene* m_scene;
    bool m_paused, m_visible;
    bool m_actorRemoved;

    Canvas(): m_pathfinding(nullptr), m_scene(nullptr), m_paused(false), m_visible(true), m_actorRemoved(false) {}

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
    friend class TUserdata<Canvas>;
    void construct(lua_State* L);
    void destroy(lua_State* L);
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int canvas_addActor(lua_State* L);
    static int canvas_removeActor(lua_State* L);
    static int canvas_clear(lua_State* L);
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
