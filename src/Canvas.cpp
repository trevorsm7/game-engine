#include <algorithm>
#include <cmath>

#include "Canvas.h"
#include "Scene.h"

Canvas::~Canvas()
{
    // NOTE: by the time this is called, it would be too late to remove any
    // remaining Actors... however, Lua will take care of this in lua_close
}

void Canvas::addActor(Actor *actor)
{
    m_toAdd.push_back(actor);

    // NOTE: wait until after update/events to do the following
    // Can't belong to more than one canvas AND prevent duplicate copies
    //if (actor->m_canvas != nullptr)
    //    actor->m_canvas->removeActor(actor);
    //actor->removeFromCanvas();

    //m_actors.push_back(actor);
    //actor->setCanvas(this);
}

void Canvas::removeActor(Actor *actor)
{
    // TODO: make sure this doesn't happen while m_actors is being iterated over
    m_actors.erase(std::remove(m_actors.begin(), m_actors.end(), actor), m_actors.end());
    actor->setCanvas(nullptr);
}

void Canvas::update(lua_State *L, float delta)
{
    if (m_paused)
        return;

    // order of update dispatch doesn't really matter; choose bottom to top
    for (auto end = m_actors.end(), it = m_actors.begin(); it != end; ++it)
        (*it)->update(L, delta);

    // done with update iterator; add new Actors to the end
    //m_actors.insert(m_actors.end(), m_toAdd.begin(), m_toAdd.end());
    for (auto end = m_toAdd.end(), it = m_toAdd.begin(); it != end; ++it)
    {
        // NOTE: we should probably make sure the remove happens right anyway
        // especially in the case of paused canvases
        (*it)->removeFromCanvas();

        m_actors.push_back(*it);
        (*it)->setCanvas(this);
    }
    m_toAdd.clear();
}

void Canvas::render(IRenderer *renderer)
{
    if (!renderer)
    {
        fprintf(stderr, "Renderer is null at Canvas::render\n");
        return;
    }

    renderer->setViewport(m_bounds.l, m_bounds.b, m_bounds.r, m_bounds.t);
    // TODO: add alternate structure to iterate in sorted order/down quadtree
    // NOTE: rendering most recently added last (on top)
    auto end = m_actors.end();
    for (auto it = m_actors.begin(); it != end; ++it)
        (*it)->render(renderer);
}

bool Canvas::mouseEvent(lua_State *L, MouseEvent& event)
{
    // Compute canvas bounds and reject if click is outside canvas
    int left = m_bounds.l < 0 ? event.w + m_bounds.l : m_bounds.l;
    int bottom = m_bounds.b < 0 ? event.h + m_bounds.b : m_bounds.b;
    int right = m_bounds.r <= 0 ? event.w + m_bounds.r : m_bounds.r;
    int top = m_bounds.t <= 0 ? event.h + m_bounds.t : m_bounds.t;
    if (event.x < left || event.x >= right || event.y < bottom || event.y >= top)
        return false;

    // Convert click into local coordinates
    int clickX = event.x - left;
    int clickY = event.y - bottom;

    // NOTE: iterate in reverse order of rendering
    auto end = m_actors.rend();
    for (auto it = m_actors.rbegin(); it != end; ++it)
    {
        // Compute actor bounds and reject if click is outside actor
        // TODO: this is only using basic transform
        int left = floor((*it)->m_transform.getX());
        int bottom = floor((*it)->m_transform.getY());
        int width = floor((*it)->m_transform.getW());
        int height = floor((*it)->m_transform.getH());
        int right = left + width;
        int top = bottom + height;
        if (clickX < left || clickX >= right || clickY < bottom || clickY >= top)
            continue;

        // TODO: try next actor down, or absorb the click?
        //return (*it)->mouseEvent(L, event.down); // absorb the click
        if ((*it)->mouseEvent(L, event.down))
            return true; // only absorb click if Actor chose to handle it
    }

    return false;
}

// ==========================================================================================
// Lua library functions
// ==========================================================================================

int Canvas::canvas_init(lua_State *L)
{
    // Push new metatable on the stack
    luaL_newmetatable(L, "Canvas");

    // Push new table to hold member functions
    static const luaL_Reg library[] =
    {
        {"addActor", canvas_addActor},
        {"removeActor", canvas_removeActor},
        {"setPaused", canvas_setPaused},
        {nullptr, nullptr}
    };
    luaL_newlib(L, library);

    // Set index of metatable to function table
    lua_pushstring(L, "__index");
    lua_insert(L, -2); // insert "__index" before function library
    lua_settable(L, -3); // metatable.__index = function library

    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, canvas_delete);
    lua_settable(L, -3);

    // Prevent metatable from being accessed through Lua
    lua_pushstring(L, "__metatable");
    lua_pushstring(L, "private");
    lua_settable(L, -3);

    // Push new table to hold global functions
    //luaL_newlib(L, /*luaL_reg*/);
    lua_newtable(L);
    // TODO: add utility functions
    lua_pushstring(L, "create");
    lua_pushcfunction(L, canvas_create);
    lua_settable(L, -3);

    // Assign global function table to global variable
    lua_setglobal(L, "Canvas");

    return 0;
}

int Canvas::canvas_create(lua_State *L)
{
    // Validate arguments
    int bounds[4] = {0, 0, 0, 0};
    if (lua_istable(L, 1))
    {
        for (int i = 0; i < 4; ++i)
        {
            lua_rawgeti(L, 1, i + 1);
            bounds[i] = static_cast<int>(luaL_checkinteger(L, -1));
            lua_pop(L, 1);
        }
    }

    //Scene *scene = static_cast<Scene*>(lua_touserdata(state, lua_upvalueindex(1)));
    // TODO: probably not best to get Scene off the registry this way; conflict if we have a Scene metatable
    lua_pushstring(L, "Scene");
    lua_gettable(L, LUA_REGISTRYINDEX);
    Scene *scene = static_cast<Scene*>(lua_touserdata(L, -1));
    luaL_argcheck(L, scene != nullptr, 1, "invalid Scene\n");

    // Create Actor userdata and construct Actor object in the allocated memory
    // NOTE: consider using a shared_ptr here instead of the Canvas object directly
    Canvas *canvas = static_cast<Canvas*>(lua_newuserdata(L, sizeof(Canvas)));
    new(canvas) Canvas(); // call the constructor on the already allocated block of memory
    canvas->m_bounds.l = bounds[0];
    canvas->m_bounds.b = bounds[1];
    canvas->m_bounds.r = bounds[2];
    canvas->m_bounds.t = bounds[3];

    //printf("created Canvas(%p)\n", canvas);

    luaL_getmetatable(L, "Canvas");
    lua_setmetatable(L, -2); // -1 is metatable, which gets popped by this call

    scene->addCanvas(canvas);

    // add userdata to registry with bare pointer as the key
    // TODO: if we ever remove the Canvas, need to clear this...
    lua_pushlightuserdata(L, canvas);
    lua_pushvalue(L, -2); // alternatively, push table w/ userdata and callbacks (if Canvas needs)
    lua_settable(L, LUA_REGISTRYINDEX);

    // userdata should be on top of Lua stack
    return 1;
}

int Canvas::canvas_delete(lua_State *L)
{
    Canvas *canvas = static_cast<Canvas*>(luaL_checkudata(L, 1, "Canvas"));
    //printf("deleting Canvas(%p)\n", canvas);
    canvas->~Canvas(); // manually call destructor before Lua calls free()

    return 0;
}

int Canvas::canvas_addActor(lua_State *L)
{
    // Validate function arguments
    Canvas *canvas = static_cast<Canvas*>(luaL_checkudata(L, 1, "Canvas"));
    Actor *actor = static_cast<Actor*>(luaL_checkudata(L, 2, "Actor"));

    // Add Actor to Canvas
    canvas->addActor(actor);

    // Add the Canvas to the registry since it is now ref'd from C++
    lua_pushlightuserdata(L, actor);
    lua_pushvalue(L, 2); // get the Actor userdata
    lua_settable(L, LUA_REGISTRYINDEX);

    return 0;
}

int Canvas::canvas_removeActor(lua_State *L)
{
    // Validate function arguments
    Canvas *canvas = static_cast<Canvas*>(luaL_checkudata(L, 1, "Canvas"));
    //luaL_argcheck(L, canvas != nullptr, 1, "invalid Canvas\n");
    Actor *actor = static_cast<Actor*>(luaL_checkudata(L, 2, "Actor"));

    // Add Actor to Canvas
    canvas->removeActor(actor);

    // TODO: don't remove from registry unless it actually belongs to the Canvas!
    // TODO: consider a "to remove" queue... will run into same problems as add

    // Remove the Actor from the registry as it is no longer ref'd from C++
    lua_pushlightuserdata(L, actor);
    lua_pushnil(L); // remove the ref to the Actor userdate
    lua_settable(L, LUA_REGISTRYINDEX);

    return 0;
}

int Canvas::canvas_setPaused(lua_State *L)
{
    // Validate function arguments
    Canvas *canvas = static_cast<Canvas*>(luaL_checkudata(L, 1, "Canvas"));
    luaL_argcheck(L, lua_isboolean(L, 2), 2, "must be boolean (true to pause)\n");

    canvas->m_paused = lua_toboolean(L, 2);

    return 0;
}
