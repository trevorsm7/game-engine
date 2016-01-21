#ifndef __CANVAS_H__
#define __CANVAS_H__

#include <vector>
#include "lua.hpp"

#include "Actor.h"
#include "Event.h"

class Scene;

class Canvas
{
    friend class Scene;

    std::vector<Actor*> m_actors;
    std::vector<Actor*> m_toAdd;
    Scene *m_scene;
    struct {int l, b, r, t;} m_bounds;
    bool m_paused;

public:
    Canvas(): m_scene(nullptr), m_bounds{0, 0, 0, 0}, m_paused(false) {}
    ~Canvas();

    void update(lua_State *L, float delta);
    void render(IRenderer *renderer);
    bool mouseEvent(lua_State *L, MouseEvent& event);

//private:
    void addActor(Actor *actor);
    void removeActor(Actor *actor);
    void setScene(Scene *scene) {m_scene = scene;}

    static int canvas_init(lua_State *L); // this at least should be public?
    static int canvas_create(lua_State *L);
    static int canvas_delete(lua_State *L);
    static int canvas_addActor(lua_State *L);
    static int canvas_removeActor(lua_State *L);
    static int canvas_setPaused(lua_State *L);
};

#endif
