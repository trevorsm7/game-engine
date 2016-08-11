#include "Scene.hpp"
#include "Canvas.hpp"
#include "Actor.hpp"
#include "IRenderer.hpp"

#include "TileMap.hpp"
#include "SpriteGraphics.hpp"
#include "TiledGraphics.hpp"
#include "AabbCollider.hpp"
#include "TiledCollider.hpp"

#include <cassert>
#include <cstring>

bool Scene::load(const char *filename)
{
    assert(m_L == nullptr);
    m_L = luaL_newstate();
    luaL_openlibs(m_L);

    // TODO: global utility functions should be protected from write and kept separate from
    // serialization. Make alternate "global" table w/ metatable to separate reads from user
    // globals and library globals.

    // Add Scene pointer to registry
    lua_pushliteral(m_L, "Scene");
    lua_pushlightuserdata(m_L, this);
    lua_rawset(m_L, LUA_REGISTRYINDEX);

    // Initialize objects and methods
    Canvas::initMetatable(m_L);
    Actor::initMetatable(m_L);
    TileIndex::initMetatable(m_L);
    TileMap::initMetatable(m_L);
    SpriteGraphics::initMetatable(m_L);
    TiledGraphics::initMetatable(m_L);
    AabbCollider::initMetatable(m_L);
    TiledCollider::initMetatable(m_L);

    lua_pushcfunction(m_L, scene_registerControl);
    lua_setglobal(m_L, "registerControl");

    lua_pushcfunction(m_L, scene_setPortraitHint);
    lua_setglobal(m_L, "setPortraitHint");

    lua_pushcfunction(m_L, scene_quit);
    lua_setglobal(m_L, "quit");

    // Execute specified script
    if (luaL_dofile(m_L, filename) != 0)
    {
        fprintf(stderr, "%s\n", lua_tostring(m_L, -1));
        return false;
    }

    return true;
}

void Scene::update(float delta)
{
    // order of update dispatch doesn't really matter; choose bottom to top
    auto end = m_canvases.end();
    for (auto it = m_canvases.begin(); it != end; ++it)
        (*it)->update(m_L, delta);

    // TODO: should we manually tell Lua to step, or wait for auto-collect?
    lua_gc(m_L, LUA_GCSTEP, 0);
}

void Scene::render(IRenderer* renderer)
{
    assert(renderer != nullptr);

    // dispatch render calls from bottom to top
    auto end = m_canvases.end();
    for (auto it = m_canvases.begin(); it != end; ++it)
        (*it)->render(renderer);
}

bool Scene::mouseEvent(MouseEvent& event)
{
    // dispatch events from top to bottom
    auto end = m_canvases.rend();
    for (auto it = m_canvases.rbegin(); it != end; ++it)
        if ((*it)->mouseEvent(m_L, event))
            return true; // stop dispatching an event when an Actor claims it

    return false;
}

bool Scene::controlEvent(ControlEvent& event)
{
    int top = lua_gettop(m_L);
    bool handled = false;

    lua_pushliteral(m_L, "controls");
    if (lua_rawget(m_L, LUA_REGISTRYINDEX) == LUA_TTABLE)
    {
        lua_pushstring(m_L, event.name);
        if (lua_rawget(m_L, -2) == LUA_TFUNCTION)
        {
            lua_pushboolean(m_L, event.down);
            if (lua_pcall(m_L, 1, 0, 0) != 0)
                printf("%s\n", lua_tostring(m_L, -1));
            else
                handled = true;
        }
    }

    lua_settop(m_L, top);
    return handled;
}

void Scene::resize(int width, int height)
{
    // order of resize dispatch doesn't really matter; choose bottom to top
    auto end = m_canvases.end();
    for (auto it = m_canvases.begin(); it != end; ++it)
        (*it)->resize(m_L, width, height);
}

Scene* Scene::checkScene(lua_State* L)
{
    // Get light userdata from registry
    lua_pushliteral(L, "Scene");
    if (lua_rawget(L, LUA_REGISTRYINDEX) != LUA_TLIGHTUSERDATA)
        luaL_error(L, "Failed to get Scene reference (non-userdata)");

    // Cast light userdata to Scene*
    auto scene = reinterpret_cast<Scene*>(lua_touserdata(L, -1));
    if (!scene) luaL_error(L, "Failed to get Scene reference (nullptr)");
    lua_pop(L, 1); // clean up the stack

    return scene;
}

void Scene::addCanvas(lua_State *L, int index)
{
    // Validate and get pointers to Scene and Canvas
    Scene* scene = Scene::checkScene(L);
    Canvas* canvas = Canvas::checkUserdata(L, index);

    // Push Canvas on layer above previously pushed Canvases
    scene->m_canvases.push_back(canvas);
    canvas->refAdded(L, index);
    canvas->m_scene = scene;
}

int Scene::scene_registerControl(lua_State* L)
{
    // Validate input
    // TODO: add user-friendly name, computer-friendly tags, view/player
    // TODO: add support for analog controls
    // TODO: add option to query state rather than wait for callback?
    Scene* scene = Scene::checkScene(L);
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

    // Notify native layer of new control
    // TODO: native layer might also want to query a list of controls
    // for example, when a new game pad is connected
    bool registered = false;
    if (scene->m_registerControlCallback)
        registered = scene->m_registerControlCallback(lua_tostring(L, 1));

    lua_pushboolean(L, registered);
    return 1;
}

int Scene::scene_setPortraitHint(lua_State* L)
{
    Scene* scene = Scene::checkScene(L);
    luaL_checktype(L, 1, LUA_TBOOLEAN);

    scene->m_isPortraitHint = lua_toboolean(L, 1);

    return 0;
}

int Scene::scene_quit(lua_State* L)
{
    Scene* scene = Scene::checkScene(L);

    if (scene->m_quitCallback)
        scene->m_quitCallback();

    return 0;
}
