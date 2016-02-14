#include "Scene.h"

void Scene::load(const char *filename)
{
    // TODO: C objects should be freed before closing Lua state
    // NOTE: might want to prevent Scene from being "loaded" more than once
    if (m_state)
        lua_close(m_state);

    m_state = luaL_newstate();
    luaL_openlibs(m_state);

    // TODO: global utility functions should be protected from write and kept separate from
    // serialization. Make alternate "global" table w/ metatable to separate reads from user
    // globals and library globals.

    // Add Scene pointer to registry
    // NOTE: could also store Scene* as function upvalue or as a full userdata type
    lua_pushstring(m_state, "Scene");
    lua_pushlightuserdata(m_state, this);
    lua_rawset(m_state, LUA_REGISTRYINDEX);

    // Initialize objects and methods
    Canvas::canvas_init(m_state);
    Actor::actor_init(m_state);

    lua_pushcfunction(m_state, scene_registerControl);
    lua_setglobal(m_state, "registerControl");

    lua_pushcfunction(m_state, scene_quit);
    lua_setglobal(m_state, "quit");

    // Register Canvas creation function with this Scene* as upvalue
    //lua_pushlightuserdata(m_state, this);
    //lua_pushcclosure(m_state, createCanvas, 1);
    //lua_setglobal(m_state, "createCanvas");

    // Execute specified script
    if (luaL_dofile(m_state, filename) != 0)
        printf("%s\n", lua_tostring(m_state, -1));
}

void Scene::addCanvas(Canvas *canvas)
{
    // push canvases on top of previous ones
    m_canvases.push_back(canvas);
    canvas->m_scene = this;
}

void Scene::update(float delta)
{
    // order of update dispatch doesn't really matter; choose bottom to top
    auto end = m_canvases.end();
    for (auto it = m_canvases.begin(); it != end; ++it)
        (*it)->update(m_state, delta);

    // TODO: should we manually tell Lua to step, or wait for auto-collect?
    lua_gc(m_state, LUA_GCSTEP, 0);
}

void Scene::render(IRenderer* renderer)
{
    if (!renderer)
    {
        fprintf(stderr, "Invalid renderer\n");
        return;
    }

    // dispatch render calls from bottom to top
    auto end = m_canvases.end();
    for (auto it = m_canvases.begin(); it != end; ++it)
        (*it)->render(renderer);
}

void Scene::mouseEvent(MouseEvent& event)
{
    // dispatch events from top to bottom
    auto end = m_canvases.rend();
    for (auto it = m_canvases.rbegin(); it != end; ++it)
        if ((*it)->mouseEvent(m_state, event))
            break; // stop dispatching an event when an Actor claims it
}

bool Scene::controlEvent(ControlEvent& event)
{
    int top = lua_gettop(m_state);
    bool handled = false;

    lua_pushliteral(m_state, "controls");
    if (lua_rawget(m_state, LUA_REGISTRYINDEX) == LUA_TTABLE)
    {
        lua_pushstring(m_state, event.name);
        if (lua_rawget(m_state, -2) == LUA_TFUNCTION)
        {
            lua_pushboolean(m_state, event.down);
            if (lua_pcall(m_state, 1, 0, 0) != 0)
                printf("%s\n", lua_tostring(m_state, -1));
            else
                handled = true;
        }
    }

    lua_settop(m_state, top);
    return handled;
}

void Scene::resize(int width, int height)
{
    // order of resize dispatch doesn't really matter; choose bottom to top
    auto end = m_canvases.end();
    for (auto it = m_canvases.begin(); it != end; ++it)
        (*it)->resize(m_state, width, height);
}

int Scene::scene_registerControl(lua_State *L)
{
    // Validate input
    // TODO: add user-friendly name, computer-friendly tags, view/player
    // TODO: add support for analog controls
    // TODO: add option to query state rather than wait for callback?
    luaL_argcheck(L, lua_isstring(L, 1), 1, "Key name must be string\n");
    luaL_argcheck(L, lua_isfunction(L, 2), 2, "Key callback must be function\n");

    lua_pushliteral(L, "controls");
    if (lua_rawget(L, LUA_REGISTRYINDEX) != LUA_TTABLE)
    {
        lua_newtable(L);
        lua_pushliteral(L, "controls");
        lua_pushvalue(L, -2); // copy ref to table; leave table on top of stack
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
    lua_pushvalue(L, 1);
    lua_pushvalue(L, 2);
    lua_rawset(L, -3);

    lua_pushliteral(L, "Scene");
    lua_rawget(L, LUA_REGISTRYINDEX);
    Scene *scene = reinterpret_cast<Scene*>(lua_touserdata(L, -1));
    luaL_argcheck(L, scene != nullptr, 1, "invalid Scene\n");

    // Notify native layer of new control
    // TODO: native layer might also want to query a list of controls
    // for example, when a new game pad is connected
    bool registered = false;
    if (scene->m_registerControlCallback)
        registered = scene->m_registerControlCallback(lua_tostring(L, 1));

    lua_pushboolean(L, registered);
    return 1;
}

int Scene::scene_quit(lua_State *L)
{
    lua_pushstring(L, "Scene");
    lua_gettable(L, LUA_REGISTRYINDEX);
    Scene *scene = reinterpret_cast<Scene*>(lua_touserdata(L, -1));
    luaL_argcheck(L, scene != nullptr, 1, "invalid Scene\n");

    if (scene->m_quitCallback)
        scene->m_quitCallback();

    return 0;
}
