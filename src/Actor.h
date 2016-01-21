#ifndef __ACTOR_H__
#define __ACTOR_H__

#include <memory>
#include "lua.hpp"

#include "Renderer.h"
#include "Component.h"
#include "Transform.h"
#include "Event.h"

class Canvas;

class Actor
{
    friend class Canvas;

    Transform m_transform;
    ComponentPtr m_graphics;
    Canvas *m_canvas;
    bool m_visible;

public:
    Actor(): m_canvas(nullptr), m_visible(true) {}
    ~Actor() {}

    //bool init() {}
    //void postInit() {}
    //void destroy() {}

    void update(lua_State* L, float delta);
    void render(IRenderer* renderer);
    bool mouseEvent(lua_State* L, bool down);//MouseEvent& event);

    void moveBy(float x, float y);

//private:
    void setGraphics(Component* graphics) {m_graphics = ComponentPtr(graphics);}
    void setCanvas(Canvas *canvas) {m_canvas = canvas;}
    void removeFromCanvas();

    // TODO: may want to move scripting stuff to another file/class
    static int actor_init(lua_State* L);
    static int actor_create(lua_State* L);
    static int actor_delete(lua_State* L);
    static int actor_index(lua_State* L);
    static int actor_newindex(lua_State* L);
    static int actor_move(lua_State* L);
    static int actor_getPosition(lua_State* L);
    static int actor_setPosition(lua_State* L);
    static int actor_setScale(lua_State* L);
    static int actor_setColor(lua_State* L);
    static int actor_setVisible(lua_State* L);
    static int actor_isVisible(lua_State* L);
};

#endif
