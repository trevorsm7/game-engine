#ifndef __CANVAS_H__
#define __CANVAS_H__

#include "Actor.h"
#include "Event.h"
#include "ICamera.h"
#include "ResourceManager.h"

#include <vector>
#include <memory>
#include "lua.hpp"

class Scene;

class Canvas
{
    friend class Scene;

    typedef std::unique_ptr<ICamera> ICameraPtr;
    typedef std::vector<Actor*> ActorVector;
    typedef ActorVector::const_iterator ActorIterator;

    std::vector<Actor*> m_actors;
    std::vector<Actor*> m_added;
    ICameraPtr m_camera;
    Scene* m_scene;
    bool m_paused, m_visible;
    bool m_actorRemoved;

public:
    Canvas(): m_scene(nullptr), m_paused(false), m_visible(true), m_actorRemoved(false) {}
    ~Canvas() {}

    ResourceManager* getResourceManager() const;

    void update(lua_State* L, float delta);
    void render(IRenderer* renderer);
    bool mouseEvent(lua_State* L, MouseEvent& event);
    void resize(lua_State* L, int width, int height) {m_camera->resize(width, height);}

private:
    void processAddedActors(lua_State *L);
    void processRemovedActors(lua_State *L);
    void updatePhysics(lua_State *L, float delta);

public:
    bool testCollision(float deltaX, float deltaY, const Actor* actor1) const;
    bool getEarliestCollision(const Actor* actor1, ActorIterator it, ActorIterator itEnd, Actor*& hit, float& start, float& end, float& normX, float& normY) const;
    bool getEarliestCollision(const Actor* actor1, Actor*& hit, float& start, float& end, float& normX, float& normY) const
        {return getEarliestCollision(actor1, m_actors.begin(), m_actors.end(), hit, start, end, normX, normY);}

    static int canvas_init(lua_State* L);
    static constexpr const char* const METATABLE = "Canvas";

private:
    static int canvas_create(lua_State* L);
    static int canvas_delete(lua_State* L);
    static int canvas_addActor(lua_State* L);
    static int canvas_removeActor(lua_State* L);
    static int canvas_clear(lua_State* L);
    static int canvas_setCenter(lua_State* L);
    static int canvas_getCollision(lua_State* L);
    static int canvas_setPaused(lua_State* L);
    static int canvas_setVisible(lua_State* L);
};

#endif
