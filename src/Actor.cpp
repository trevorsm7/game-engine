#include "Actor.h"
#include "Canvas.h"
#include "SpriteGraphics.h"
#include "AabbCollider.h"
#include "TiledGraphics.h"
#include "TiledCollider.h"
#include "Physics.h"

#include <limits>

ResourceManager* Actor::getResourceManager() const
{
    if (m_canvas)
        return m_canvas->getResourceManager();

    return nullptr;
}

void Actor::update(lua_State* L, float delta)
{
    lua_pushnumber(L, delta);
    pcall(L, "update", 1, 0);

    if (m_graphics)
        m_graphics->update(delta);

    // NOTE this should be, for example, related to resizing bounds to reflect animation; not related to collisions
    if (m_collider)
        m_collider->update(delta);
}

void Actor::render(IRenderer* renderer)
{
    // return right away if not visible?
    renderer->pushModelTransform(m_transform);
    if (m_graphics)
        m_graphics->render(renderer);
    // TODO: render children? need to check if visible?
    renderer->popModelTransform();
}

bool Actor::mouseEvent(lua_State* L, bool down)//MouseEvent& event)
{
    bool handled = false;

    lua_pushboolean(L, down);
    if (pcall(L, "mouse", 1, 1))
    {
        handled = lua_isboolean(L, -1) && lua_toboolean(L, -1);
        lua_pop(L, 1);
    }

    return handled;
}

void Actor::collideEvent(lua_State* L, Actor* with)
{
    // Push other Actor's full userdata on stack
    lua_pushlightuserdata(L, with);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_isuserdata(L, -1)) // check metatable?
    {
        // Reject or make the pcall with nil?
        lua_pop(L, 1);
        return;
    }
    pcall(L, "collided", 1, 0);
}

bool Actor::testMouse(float x, float y) const
{
    if (!m_graphics)
        return false;

    return m_graphics->testBounds(x, y);
}

bool Actor::testCollision(float x, float y) const
{
    // NOTE: should invisible actors still be collidable? maybe
    if (!m_collider)
        return false;

    return m_collider->testCollision(x, y);
}

// TODO: might move this to Canvas, but would want to move refCount as well
// ...could possibly stick refCount in registry instead
void Actor::refAdded(lua_State* L, int index)
{
    // Add userdata to the registry while it is ref'd by engine
    if (m_refCount++ == 0)
    {
        //printf("Actor(%p) added to registry\n", this);
        lua_pushlightuserdata(L, this);
        lua_pushvalue(L, index); // push the Actor userdata
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
}

void Actor::refRemoved(lua_State* L)
{
    // Remove from registry if reference count drops to zero
    if (--m_refCount == 0)
    {
        //printf("Actor(%p) removed from registry\n", this);
        lua_pushlightuserdata(L, this);
        lua_pushnil(L);
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
}

bool Actor::pcall(lua_State* L, const char* method, int in, int out)
{
    // Push full userdata on stack ([in] udata)
    lua_pushlightuserdata(L, this);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_isuserdata(L, -1)) // check metatable?
    {
        fprintf(stderr, "pcall(%s): bad userdata (%p)\n", method, this);
        lua_pop(L, in + 1);
        return false;
    }

    // Push function on stack ([in] udata uvalue function)
    lua_getuservalue(L, -1);
    lua_pushstring(L, method); // TODO: this can fail (out of memory)
    if (lua_rawget(L, -2) != LUA_TFUNCTION)
    {
        // NOTE: this is not an error so don't spam error messages!
        //fprintf(stderr, "pcall(%s): not a function \n", method);
        lua_pop(L, in + 3);
        return false;
    }

    // Reorder stack (function udata [in])
    lua_insert(L, -(in + 3)); // insert function before args
    lua_pop(L, 1); // remove the uservalue from the stack
    lua_insert(L, -(in + 1)); // insert udata before args

    // Do a protected call; pops function, udata, and args
    if (lua_pcall(L, in + 1, out, 0) != 0)
    {
        fprintf(stderr, "pcall(%s): script error\n%s\n", method, lua_tostring(L, -1));
        lua_pop(L, 1); // remove the error string from the stack
        return false;
    }

    return true;
}

// =============================================================================
// Lua library functions
// =============================================================================

int Actor::actor_init(lua_State* L)
{
    // Push new metatable on the stack
    luaL_newmetatable(L, METATABLE);

    // Push new table to hold member functions
    static const luaL_Reg library[] =
    {
        {"getCanvas", actor_getCanvas},
        {"getPosition", actor_getPosition},
        {"setPosition", actor_setPosition},
        {"setScale", actor_setScale},
        {"setColor", actor_setColor},
        {"setVisible", actor_setVisible},
        {"isVisible", actor_isVisible},
        {"setCollidable", actor_setCollidable},
        {"testCollision", actor_testCollision},
        //{"getEarliestCollision", actor_getEarliestCollision},
        {"setVelocity", actor_setVelocity},
        {"getVelocity", actor_getVelocity},
        {"addAcceleration", actor_addAcceleration},
        {nullptr, nullptr}
    };
    luaL_newlib(L, library);

    // Index goes to C function with function table as closure
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2); // push copy of function library
    lua_pushcclosure(L, actor_index, 1);
    lua_rawset(L, -4); // metatable.__index = getter method

    // Writes go to C function with function library as closure
    // NOTE: this will pop the function library leaving the metatable on top
    lua_pushstring(L, "__newindex");
    lua_insert(L, -2); // swap "__newindex" and function table
    lua_pushcclosure(L, actor_newindex, 1);
    lua_rawset(L, -3); // metatable.__newindex = setter method

    // Make sure to call destructor when the Actor is GC'd
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, actor_delete);
    lua_rawset(L, -3); // meta.__gc = actor_gcNum

    // Prevent metatable from being accessed through Lua
    lua_pushstring(L, "__metatable");
    lua_pushstring(L, "private");
    lua_rawset(L, -3);

    // TODO: maybe better to just store functions globally? Like createActor instead
    // Push new table to hold global functions
    //luaL_newlib(L, /*luaL_reg*/);
    lua_newtable(L);
    // TODO: add utility functions
    lua_pushstring(L, "create");
    lua_pushcfunction(L, actor_create);
    lua_rawset(L, -3);

    // Assign global function table to global variable
    lua_setglobal(L, "Actor");

    return 0;
}

int Actor::actor_create(lua_State* L)
{
    // Create Actor userdata and construct Actor object in the allocated memory
    // NOTE: consider using a shared_ptr here instead of the Actor object directly
    Actor* actor = reinterpret_cast<Actor*>(lua_newuserdata(L, sizeof(Actor)));
    new(actor) Actor(); // call the constructor on the already allocated block of memory

    //printf("created Actor(%p)\n", actor);

    // Get Actor metatable and assign it to the new Actor
    luaL_getmetatable(L, METATABLE);
    lua_setmetatable(L, -2); // -1 is metatable, which gets popped by this call

    // Add empty table as uservalue for storage of runtime variables
    lua_newtable(L);
    lua_setuservalue(L, -2);

    // Create the Actor object
    // TODO: try and refactor this procedure for getting params
    // TODO: what about looping of given params instead of checking all?
    if (lua_istable(L, 1))
    {
        int top = lua_gettop(L);

        lua_pushliteral(L, "sprite");
        if (lua_rawget(L, 1) == LUA_TSTRING)
        {
            // TODO should log an error if the type is non-string and non-nil
            SpriteGraphics* graphics = new SpriteGraphics(actor, lua_tostring(L, -1));
            actor->m_graphics = IGraphicsPtr(graphics); // take responsibility for ptr

            lua_pushliteral(L, "collider");
            if (lua_rawget(L, 1) == LUA_TBOOLEAN && lua_toboolean(L, -1))
                actor->m_collider = IColliderPtr(new AabbCollider(actor));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        // TODO: make graphics components exclusive (or allow layering multiple?)
        lua_pushliteral(L, "tiles");
        if (lua_rawget(L, 1) == LUA_TSTRING)
        {
            const char* tileMap = lua_tostring(L, -1);
            TiledGraphics* graphics = new TiledGraphics(actor, tileMap);
            actor->m_graphics = IGraphicsPtr(graphics); // take responsibility for ptr

            lua_pushliteral(L, "collider");
            if (lua_rawget(L, 1) == LUA_TBOOLEAN && lua_toboolean(L, -1))
                actor->m_collider = IColliderPtr(new TiledCollider(actor, tileMap));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);

        if (!actor->m_graphics)
        {
            lua_pushliteral(L, "collider");
            if (lua_rawget(L, 1) == LUA_TBOOLEAN && lua_toboolean(L, -1))
                actor->m_collider = IColliderPtr(new AabbCollider(actor));
            lua_pop(L, 1);
        }

        lua_pushliteral(L, "physics");
        if (lua_rawget(L, 1) == LUA_TBOOLEAN && lua_toboolean(L, -1))
        {
            float mass = 1.f;
            lua_pushliteral(L, "mass");
            if (lua_rawget(L, 1) == LUA_TNUMBER)
                mass = lua_tonumber(L, -1);

            float cor = 1.f;
            lua_pushliteral(L, "cor");
            if (lua_rawget(L, 1) == LUA_TNUMBER)
                cor = lua_tonumber(L, -1);

            float cof = 0.f;
            lua_pushliteral(L, "cof");
            if (lua_rawget(L, 1) == LUA_TNUMBER)
                cof = lua_tonumber(L, -1);

            actor->m_physics = PhysicsPtr(new Physics(mass, cor, cof));
            lua_pop(L, 2);
        }
        lua_pop(L, 1);

        lua_pushliteral(L, "color");
        if (lua_rawget(L, 1) == LUA_TTABLE && actor->m_graphics)
        {
            lua_rawgeti(L, -1, 1); // should return LUA_TNUMBER
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            float r = lua_tonumber(L, -3);
            float g = lua_tonumber(L, -2);
            float b = lua_tonumber(L, -1);
            lua_pop(L, 3);

            actor->m_graphics->setColor(r, g, b);
        }
        lua_pop(L, 1);

        lua_pushliteral(L, "layer");
        if (lua_rawget(L, 1) == LUA_TNUMBER)
        {
            actor->setLayer(lua_tointeger(L, -1));
        }
        lua_pop(L, 1);

        lua_settop(L, top);
    }

    // NOTE: do not add Actor to registry until it is hooked into C++ (Canvas)

    return 1;
}

int Actor::actor_delete(lua_State* L)
{
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    //printf("deleting Actor(%p)\n", actor);
    actor->~Actor(); // manually call destructor before Lua calls free()

    return 0;
}

int Actor::actor_index(lua_State* L)
{
    // Validate Actor userdata
    luaL_checkudata(L, 1, METATABLE); // don't need userdata, just check that it's valid
    //luaL_checktype(L, lua_upvalueindex(1), LUA_TTABLE); // function table

    // First check if the key is in the function table
    lua_pushvalue(L, 2);
    if (lua_rawget(L, lua_upvalueindex(1)) != LUA_TNIL)
        return 1;

    // Get the uservalue from Actor and index it with the 2nd argument
    lua_getuservalue(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2); // if type is nil, return it anyway

    return 1;
}

int Actor::actor_newindex(lua_State* L)
{
    // Validate Actor userdata
    luaL_checkudata(L, 1, METATABLE); // don't need userdata, just check that it's valid
    //luaL_checktype(L, lua_upvalueindex(1), LUA_TTABLE); // function table

    // First check if the key is in the function table
    lua_pushvalue(L, 2);
    if (lua_rawget(L, lua_upvalueindex(1)) != LUA_TNIL)
    {
        printf("Attempt to overwrite reserved name \"%s\" on Actor\n", lua_tostring(L, 2));
        return 0;
    }

    // Get the uservalue from Actor and index it with the 2nd argument
    lua_getuservalue(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);

    return 0;
}

int Actor::actor_getCanvas(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));

    if (actor->m_canvas != nullptr)
    {
        // TODO: query Canvas to push its userdata instead?
        lua_pushlightuserdata(L, actor->m_canvas);
        lua_rawget(L, LUA_REGISTRYINDEX);
    }
    else
        lua_pushnil(L);

    return 1;
}

int Actor::actor_getPosition(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));

    lua_pushnumber(L, actor->m_transform.getX());
    lua_pushnumber(L, actor->m_transform.getY());
    // TODO: return Z?

    return 2;
}

int Actor::actor_setPosition(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    actor->m_transform.setX(x);
    actor->m_transform.setY(y);

    lua_pushvalue(L, 1);
    return 1;
}

int Actor::actor_setScale(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    float w = static_cast<float>(luaL_checknumber(L, 2));
    float h = static_cast<float>(luaL_checknumber(L, 3));

    actor->m_transform.setW(w);
    actor->m_transform.setH(h);

    lua_pushvalue(L, 1);
    return 1;
}

int Actor::actor_setColor(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    luaL_checktype(L, 2, LUA_TTABLE);
    // TODO: needs better validation

    if (lua_istable(L, 2))
    {
        lua_rawgeti(L, 2, 1); // should return LUA_TNUMBER
        float red = lua_tonumber(L, -1);
        lua_rawgeti(L, 2, 2);
        float green = lua_tonumber(L, -1);
        lua_rawgeti(L, 2, 3);
        float blue = lua_tonumber(L, -1);
        lua_pop(L, 3);

        if (actor->m_graphics)
            actor->m_graphics->setColor(red, green, blue);
    }

    lua_pushvalue(L, 1);
    return 1;
}

int Actor::actor_setVisible(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    //luaL_argcheck(L, (actor->m_graphics != nullptr), 1, "must have graphics component\n");
    luaL_argcheck(L, lua_isboolean(L, 2), 2, "must be boolean (true to pause)\n");

    if (actor->m_graphics)
        actor->m_graphics->setVisible(lua_toboolean(L, 2));

    lua_pushvalue(L, 1);
    return 1;
}

int Actor::actor_isVisible(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    //luaL_argcheck(L, (actor->m_graphics != nullptr), 1, "must have graphics component\n");

    lua_pushboolean(L, actor->m_graphics && actor->m_graphics->isVisible());

    return 1;
}

int Actor::actor_setCollidable(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    //luaL_argcheck(L, (actor->m_collider != nullptr), 1, "must have collision component\n");
    luaL_argcheck(L, lua_isboolean(L, 2), 2, "must be boolean\n");

    if (actor->m_collider != nullptr)
        actor->m_collider->setCollidable(lua_toboolean(L, 2));

    lua_pushvalue(L, 1);
    return 1;
}

int Actor::actor_testCollision(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    float deltaX = static_cast<float>(luaL_checknumber(L, 2));
    float deltaY = static_cast<float>(luaL_checknumber(L, 3));

    bool result = false;
    if (actor->m_collider && actor->m_canvas)
        result = actor->m_canvas->testCollision(deltaX, deltaY, actor);

    lua_pushboolean(L, result);

    return 1;
}

/*int Actor::actor_getEarliestCollision(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));

    Actor* hit = nullptr;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::max();;

    bool result = false;
    if (actor->m_collider && actor->m_canvas)
        result = actor->m_canvas->getEarliestCollision(actor, hit, start, end);

    lua_pushboolean(L, result);
    if (hit != nullptr)
    {
        lua_pushlightuserdata(L, hit);
        lua_rawget(L, LUA_REGISTRYINDEX);
    }
    else
        lua_pushnil(L);
    lua_pushnumber(L, start);
    lua_pushnumber(L, end);

    return 4;
}*/

int Actor::actor_setVelocity(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    if (actor->m_physics)
    {
        actor->m_physics->setVelX(x);
        actor->m_physics->setVelY(y);
    }

    lua_pushvalue(L, 1);
    return 1;
}

int Actor::actor_getVelocity(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));

    Physics* physics = actor->m_physics.get();
    if (physics)
    {
        lua_pushnumber(L, physics->getVelX());
        lua_pushnumber(L, physics->getVelY());
    }
    else
    {
        lua_pushnumber(L, 0.);
        lua_pushnumber(L, 0.);
    }

    return 2;
}

int Actor::actor_addAcceleration(lua_State* L)
{
    // Validate function arguments
    Actor* actor = reinterpret_cast<Actor*>(luaL_checkudata(L, 1, METATABLE));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    if (actor->m_physics)
    {
        actor->m_physics->addAccX(x);
        actor->m_physics->addAccY(y);
    }

    lua_pushvalue(L, 1);
    return 1;
}
