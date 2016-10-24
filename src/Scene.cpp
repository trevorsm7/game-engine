#include "Scene.hpp"
#include "Canvas.hpp"
#include "Actor.hpp"
#include "IRenderer.hpp"
#include "Serializer.hpp"
#include "IAudio.hpp"

#include "TileMap.hpp"
#include "SpriteGraphics.hpp"
#include "TiledGraphics.hpp"
#include "AabbCollider.hpp"
#include "TiledCollider.hpp"
#include "TiledPathing.hpp"

#include <cassert>
#include <cstring>
#include <cstdio>
#include <limits>
#include "lua.hpp"

static void copyglobal(lua_State* L, const char* name, int src, int dst)
{
    lua_pushstring(L, name);
    lua_pushvalue(L, -1);
    lua_rawget(L, (src < 0 ? src - 2 : src));

    int type = lua_type(L, -1);
    switch (type)
    {
    case LUA_TSTRING:
    case LUA_TNUMBER:
    case LUA_TBOOLEAN:
    case LUA_TFUNCTION:
    case LUA_TUSERDATA:
    case LUA_TLIGHTUSERDATA:
        lua_rawset(L, (dst < 0 ? dst - 2 : dst));
        break;
    case LUA_TTABLE:
        // Copy into a read-only userdata
        lua_newuserdata(L, 0);
        lua_newtable(L); // metatable

        // Prevent metatable from being accessed directly
        lua_pushliteral(L, "__metatable");
        lua_pushvalue(L, -5); // name
        lua_rawset(L, -3);

        // Push methods table as index of class object
        // TODO need to make a deep copy if there are nested tables
        lua_pushliteral(L, "__index");
        lua_rotate(L, -4, -1); // move table to top
        lua_rawset(L, -3);

        // Set the metatable and move to dest
        lua_setmetatable(L, -2);
        lua_rawset(L, (dst < 0 ? dst - 2 : dst));
        break;
    default:
        fprintf(stderr, "Attempt to copy key %s with type %s\n", name, lua_typename(L, type));
        lua_pop(L, 2);
        break;
    }
}

Scene::~Scene()
{
    // TODO remove Canvas/other references; will be deleted anyway when closing Lua state, but would be nice to do?
    if (m_L)
        lua_close(m_L);
}

bool Scene::load(const char *filename)
{
    assert(m_L == nullptr);
    m_L = luaL_newstate();

    // Add Scene pointer to registry
    lua_pushliteral(m_L, "Scene");
    lua_pushlightuserdata(m_L, this);
    lua_rawset(m_L, LUA_REGISTRYINDEX);

    // Load only selected libraries
    //luaL_openlibs(m_L);
    static constexpr const luaL_Reg libs[] =
    {
        {"_G", luaopen_base},
        //{LUA_LOADLIBNAME, luaopen_package},
        //{LUA_COLIBNAME, luaopen_coroutine},
        {LUA_TABLIBNAME, luaopen_table},
        //{LUA_IOLIBNAME, luaopen_io},
        //{LUA_OSLIBNAME, luaopen_os},
        {LUA_STRLIBNAME, luaopen_string},
        {LUA_MATHLIBNAME, luaopen_math},
        {LUA_UTF8LIBNAME, luaopen_utf8},
        //{LUA_DBLIBNAME, luaopen_debug},
        {LUA_BITLIBNAME, luaopen_bit32},
        {NULL, NULL}
    };
    const luaL_Reg *lib;
    for (lib = libs; lib->func; lib++)
    {
        luaL_requiref(m_L, lib->name, lib->func, 1);
        lua_pop(m_L, 1);
    }

    // TODO seed math.random since we're taking away os.time

    // ==== Create table for read-only globals ====
    int top = lua_gettop(m_L);
    lua_newtable(m_L);

    // Get the initial global table to copy from
    lua_pushglobaltable(m_L);
    copyglobal(m_L, "assert", -1, -2);
    copyglobal(m_L, "error", -1, -2);
    copyglobal(m_L, "getmetatable", -1, -2);
    copyglobal(m_L, "ipairs", -1, -2);
    copyglobal(m_L, "load", -1, -2); // TODO replace with custom implementation
    copyglobal(m_L, "next", -1, -2);
    copyglobal(m_L, "pairs", -1, -2);
    copyglobal(m_L, "print", -1, -2); // TODO replace with log to in-game console
    copyglobal(m_L, "rawequal", -1, -2);
    copyglobal(m_L, "rawget", -1, -2);
    copyglobal(m_L, "rawlen", -1, -2);
    copyglobal(m_L, "rawset", -1, -2);
    copyglobal(m_L, "select", -1, -2);
    copyglobal(m_L, "setmetatable", -1, -2);
    copyglobal(m_L, "tonumber", -1, -2);
    copyglobal(m_L, "tostring", -1, -2);
    copyglobal(m_L, "type", -1, -2);

    copyglobal(m_L, "math", -1, -2);
    copyglobal(m_L, "bit32", -1, -2);
    copyglobal(m_L, "string", -1, -2);
    copyglobal(m_L, "table", -1, -2);
    copyglobal(m_L, "utf8", -1, -2);
    lua_pop(m_L, 1);

    // Initialize objects and methods
    Canvas::initMetatable(m_L);
    Actor::initMetatable(m_L);
    TileIndex::initMetatable(m_L);
    TileMap::initMetatable(m_L);
    SpriteGraphics::initMetatable(m_L);
    TiledGraphics::initMetatable(m_L);
    AabbCollider::initMetatable(m_L);
    TiledCollider::initMetatable(m_L);
    TiledPathing::initMetatable(m_L);

    lua_pushliteral(m_L, "inf");
    lua_pushnumber(m_L, std::numeric_limits<lua_Number>::infinity());
    lua_rawset(m_L, -3);

    lua_pushliteral(m_L, "nan");
    lua_pushnumber(m_L, std::numeric_limits<lua_Number>::quiet_NaN());
    lua_rawset(m_L, -3);

    lua_pushliteral(m_L, "addCanvas");
    lua_pushcfunction(m_L, scene_addCanvas);
    lua_rawset(m_L, -3);

    lua_pushliteral(m_L, "loadClosure");
    lua_pushcfunction(m_L, scene_loadClosure);
    lua_rawset(m_L, -3);

    lua_pushliteral(m_L, "saveState");
    lua_pushcfunction(m_L, scene_saveState);
    lua_rawset(m_L, -3);

    lua_pushliteral(m_L, "playSample");
    lua_pushcfunction(m_L, scene_playSample);
    lua_rawset(m_L, -3);

    lua_pushliteral(m_L, "registerControl");
    lua_pushcfunction(m_L, scene_registerControl);
    lua_rawset(m_L, -3);

    lua_pushliteral(m_L, "setPortraitHint");
    lua_pushcfunction(m_L, scene_setPortraitHint);
    lua_rawset(m_L, -3);

    lua_pushliteral(m_L, "quit");
    lua_pushcfunction(m_L, scene_quit);
    lua_rawset(m_L, -3);

    // ==== Create userdata wrapper for globals ====
    lua_newuserdata(m_L, 0);
    lua_pushvalue(m_L, -1); // Replace globals table; old one should be freed?
    lua_rawseti(m_L, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

    // Set reference to global table in read-only globals
    lua_pushliteral(m_L, "_G");
    lua_pushvalue(m_L, -2);
    lua_rawset(m_L, -4); // <-- read-only globals --

    // ==== Create the read-write table ====
    lua_newtable(m_L);

    // Create metatable for read-write which indexes read-only
    lua_newtable(m_L);

    lua_pushliteral(m_L, "__index");
    lua_pushvalue(m_L, -5); // <-- read-only globals --
    lua_rawset(m_L, -3);

    lua_setmetatable(m_L, -2);

    // ==== Create metatable for globals wrapper ====
    lua_newtable(m_L);

    // Index from read-write table (which indexes from read-only)
    lua_pushliteral(m_L, "__index");
    lua_rotate(m_L, -3, -1); // rotate read-write table to the top
    lua_rawset(m_L, -3);

    // Write to table using helper function
    lua_pushliteral(m_L, "__newindex");
    lua_pushcfunction(m_L, scene_writeGlobal);
    lua_rawset(m_L, -3);

    lua_pushliteral(m_L, "__metatable");
    lua_pushliteral(m_L, "read-only");
    lua_rawset(m_L, -3);

    lua_setmetatable(m_L, -2); // <-- globals wrapper --

    // Pop global tables
    assert(lua_gettop(m_L) == top + 2);
    lua_settop(m_L, top);

    // ==== Load the user script ====
    if (luaL_loadfile(m_L, filename) != 0)
    {
        fprintf(stderr, "%s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;
    }

    // Save the global chunk so we can check the _ENV upvalue later
    lua_pushliteral(m_L, "GLOBAL_CHUNK");
    lua_pushvalue(m_L, -2);
    lua_rawset(m_L, LUA_REGISTRYINDEX);

    setWatchdog(2000);
    int ret = lua_pcall(m_L, 0, 0, 0);
    clearWatchdog();

    if (ret != 0)
    {
        fprintf(stderr, "%s\n", lua_tostring(m_L, -1));
        lua_pop(m_L, 1);
        return false;
    }

    return true;
}

void Scene::setWatchdog(int millis)
{
    using namespace std::chrono;
    m_watchdog = steady_clock::now();
    m_watchdogTotal = millis;
    m_watchdogCount = 0;
    lua_sethook(m_L, hook_watchdog, LUA_MASKCOUNT, 2000);
}

void Scene::clearWatchdog()
{
    using namespace std::chrono;
    lua_sethook(m_L, nullptr, 0, 0);

    /*auto current = steady_clock::now();
    auto elapsed = duration_cast<milliseconds>(current - m_watchdog);
    if (elapsed.count() > 0)
        fprintf(stderr, "watchdog woke %d times over %d ms\n", m_watchdogCount, int(elapsed.count()));*/
}

void Scene::hook_watchdog(lua_State* L, lua_Debug* ar)
{
    using namespace std::chrono;
    Scene* scene = checkScene(L);
    ++(scene->m_watchdogCount);

    //if (ar->event != LUA_HOOKCOUNT)
    //    fprintf(stderr, "===watchdog with event %d===\n", ar->event);

    auto current = steady_clock::now();
    auto elapsed = duration_cast<milliseconds>(current - scene->m_watchdog);

    if (elapsed.count() > scene->m_watchdogTotal)
        luaL_error(L, "watchdog reset after %d milliseconds", scene->m_watchdogTotal);
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

void Scene::playAudio(IAudio* audio)
{
    // TODO replace sample list with list of AudioSource classes
    for (auto& sample : m_tempAudioList)
        audio->playSample(sample);

    m_tempAudioList.clear();
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
    lua_rawget(L, LUA_REGISTRYINDEX);
    assert(lua_type(L, -1) == LUA_TLIGHTUSERDATA);

    // Cast light userdata to Scene*
    auto scene = reinterpret_cast<Scene*>(lua_touserdata(L, -1));
    assert(scene != nullptr);
    lua_pop(L, 1);

    return scene;
}

int Scene::scene_addCanvas(lua_State *L)
{
    // Validate and get pointers to Scene and Canvas
    Scene* scene = Scene::checkScene(L);
    Canvas* canvas = Canvas::checkUserdata(L, 1);

    // Push Canvas on layer above previously pushed Canvases
    scene->m_canvases.push_back(canvas);
    canvas->refAdded(L, 1);
    canvas->m_scene = scene;

    return 0;
}

int Scene::scene_loadClosure(lua_State* L)
{
    int args = lua_gettop(L);
    luaL_checktype(L, 1, LUA_TSTRING);

    size_t size;
    const char* buffer = luaL_tolstring(L, 1, &size);
    luaL_loadbuffer(L, buffer, size, nullptr);

    // Clear _ENV if no args are provided
    // TODO just pass error if not enough args are passed
    if (args == 1)
    {
        lua_pushnil(L);
        if (!lua_setupvalue(L, -2, 1))
            lua_pop(L, 1);
    }

    for (int i = 1; i < args; ++i)
    {
        lua_pushvalue(L, i + 1);
        if (!lua_setupvalue(L, -2, i))
        {
            // TODO error if too many args?
            lua_pop(L, 1);
            break;
        }
    }

    return 1;
}

int Scene::scene_saveState(lua_State* L)
{
    Scene* scene = Scene::checkScene(L);

    // TODO make sure this object is safe from lua_errors (memory leak!)
    Serializer serializer;

    // Push global tables
    int top = lua_gettop(L);
    lua_pushglobaltable(L);
    const void* G = lua_topointer(L, -1);
    luaL_getmetafield(L, -1, "__index"); // read-write table
    assert(lua_gettop(L) == top + 2);

    // Populate read-only globals
    luaL_getmetafield(L, -1, "__index"); // read-only table
    assert(lua_gettop(L) == top + 3);
    serializer.populateGlobals(G, "", L, -1);
    lua_pop(L, 1);

    // Serialize the _ENV upvalue if it differs from _G
    lua_pushliteral(L, "GLOBAL_CHUNK");
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_getupvalue(L, -1, 1); // get _ENV
    assert(lua_gettop(L) == top + 4);
    if (lua_topointer(L, -1) != G)
        serializer.serializeEnv(L, -1);
    lua_pop(L, 2);

    // Serialize read-write globals
    serializer.serializeSubtable(nullptr, "", L, -1);
    lua_settop(L, top);

    // Serialize all Canvases with an addCanvas setter
    for (Canvas*& canvas : scene->m_canvases)
    {
        canvas->pushUserdata(L);
        serializer.serializeSetter("addCanvas", L, {-1});
        lua_pop(L, 1);
    }

    // Serialize all controls with a registerControl setter
    lua_pushliteral(L, "controls");
    if (lua_rawget(L, LUA_REGISTRYINDEX) != LUA_TNIL)
    {
        assert(lua_type(L, -1) == LUA_TTABLE);
        lua_pushnil(L);
        while (lua_next(L, -2))
        {
            serializer.serializeSetter("registerControl", L, {-2, -1});
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);

    // Serialize portrait hint, if set
    if (scene->m_isPortraitHint)
    {
        lua_pushboolean(L, scene->m_isPortraitHint);
        serializer.serializeSetter("setPortraitHint", L, {-1});
        lua_pop(L, 1);
    }

    serializer.print();

    return 0;
}

int Scene::scene_writeGlobal(lua_State* L)
{
    int top = lua_gettop(L);
    luaL_getmetafield(L, 1, "__index"); // read-write table
    assert(lua_gettop(L) == top + 1);

    luaL_getmetafield(L, -1, "__index"); // read-only table
    assert(lua_gettop(L) == top + 2);

    // Fail if key exists in read-only globals table
    lua_pushvalue(L, 2);
    if (lua_rawget(L, -2) != LUA_TNIL)
        luaL_error(L, "global value %s is read-only", lua_tostring(L, 2));

    // Write the new value to the user table
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, top + 1);

    return 0;
}

// TODO remove this in favor of AudioSource class
int Scene::scene_playSample(lua_State* L)
{
    // Validate input
    Scene* scene = Scene::checkScene(L);
    const char* filename = luaL_checkstring(L, 1);

    scene->m_tempAudioList.push_back(filename);

    return 0;
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
