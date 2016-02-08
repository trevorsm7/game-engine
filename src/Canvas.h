#ifndef __CANVAS_H__
#define __CANVAS_H__

#include "Actor.h"
#include "Event.h"
#include "ICamera.h"

#include <vector>
#include <memory>
#include "lua.hpp"

class Scene;

class Canvas
{
    friend class Scene;

    typedef std::unique_ptr<ICamera> ICameraPtr;

    std::vector<Actor*> m_actors;
    std::vector<Actor*> m_added;
    ICameraPtr m_camera;
    Scene* m_scene;
    struct {int l, b, r, t;} m_bounds;
    bool m_paused, m_visible;

public:
    Canvas(): m_scene(nullptr), m_bounds{0, 0, 0, 0}, m_paused(false), m_visible(true) {}
    ~Canvas();

    void update(lua_State* L, float delta);
    void render(IRenderer* renderer);
    bool mouseEvent(lua_State* L, MouseEvent& event);
    void resize(lua_State* L, int width, int height);

    static int canvas_init(lua_State* L);

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
