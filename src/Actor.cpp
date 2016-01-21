#include "Actor.h"
#include "Renderer.h"
#include "GraphicsComponent.h"
#include "Canvas.h"

void Actor::update(lua_State* L, float delta)
{
    // TODO: is this the best way to call Lua methods?
    // TODO: as is, this could be refactored (with varargs???)
    // NOTE: do not do Actor validation or anything that can throw error outside of pcall!!
    int top = lua_gettop(L);
    //printf("getting userdata for %p\n", this);
    lua_pushlightuserdata(L, this);
    lua_rawget(L, LUA_REGISTRYINDEX); // safe to call (unlike lua_gettable)
    if (!lua_isuserdata(L, -1))
    {
        printf("trying to update Actor(%p) - not userdata\n", this);
        lua_settop(L, top); // pop the uservalue and userda
        return;
    }
    lua_getuservalue(L, -1);
    lua_pushstring(L, "update"); // <-- VARARGS
    if (lua_rawget(L, -2) == LUA_TFUNCTION)
    {
        lua_pushvalue(L, -3); // push the userdata
        lua_pushnumber(L, delta); // <-- VARARGS
        if (lua_pcall(L, 2, 0, 0) != 0) // pops function + args <-- VARARGS
            printf("%s\n", lua_tostring(L, -1));
    }
    lua_settop(L, top); // pop the uservalue and userdata

    if (m_graphics)
        m_graphics->update(L, delta);
}

void Actor::render(IRenderer* renderer)
{
    if (!m_visible)
        return;

    if (!renderer)
    {
        fprintf(stderr, "Error: renderer is null\n");
    }

    renderer->pushModelTransform(m_transform);
    // NOTE: the following is potentially unnecessary, but correct if we are unsure of type;
    // we could iterate over a vector of components this way
    if (GraphicsComponent* graphics = dynamic_cast<GraphicsComponent*>(m_graphics.get()))
        graphics->render(renderer);
    // TODO: render children?
    renderer->popModelTransform();
}

bool Actor::mouseEvent(lua_State* L, bool down)//MouseEvent& event)
{
    bool handled = false;
    int top = lua_gettop(L);

    lua_pushlightuserdata(L, this);
    lua_rawget(L, LUA_REGISTRYINDEX); // safe to call (unlike lua_gettable)
    lua_getuservalue(L, -1);
    lua_pushstring(L, "mouse"); // <-- VARARGS
    if (lua_rawget(L, -2) == LUA_TFUNCTION)
    {
        lua_pushvalue(L, -3); // push the userdata
        lua_pushboolean(L, down); // <-- VARARGS
        if (lua_pcall(L, 2, 1, 0) != 0) // pops function + args <-- VARARGS
            printf("%s\n", lua_tostring(L, -1));
        else
            handled = lua_isboolean(L, -1) && lua_toboolean(L, -1);
    }

    lua_settop(L, top); // pop the uservalue and userdata
    return handled;
}

void Actor::moveBy(float x, float y)
{
    //printf("Actor(%p)::moveBy\n", this);
    m_transform.moveBy(x, y);
}

void Actor::removeFromCanvas()
{
    // TODO: this function is weird; responsibility split between Canvas and Actor
    if (m_canvas)
        m_canvas->removeActor(this);
    //m_canvas = nullptr;
}

// ==========================================================================================
// Lua library functions
// ==========================================================================================

int Actor::actor_init(lua_State* L)
{
    // Push new metatable on the stack
    luaL_newmetatable(L, "Actor");

    // Push new table to hold member functions
    static const luaL_Reg library[] =
    {
        {"move", actor_move},
        {"getPosition", actor_getPosition},
        {"setPosition", actor_setPosition},
        {"setScale", actor_setScale},
        {"setColor", actor_setColor},
        {"setVisible", actor_setVisible},
        {"isVisible", actor_isVisible},
        {nullptr, nullptr}
    };
    luaL_newlib(L, library);

    // Index goes to C function with function table as closure
    lua_pushstring(L, "__index");
    lua_pushvalue(L, -2); // push copy of function library
    lua_pushcclosure(L, actor_index, 1);
    lua_settable(L, -4); // metatable.__index = getter method

    // Writes go to C function with function library as closure
    // NOTE: this will pop the function library leaving the metatable on top
    lua_pushstring(L, "__newindex");
    lua_insert(L, -2); // swap "__newindex" and function table
    lua_pushcclosure(L, actor_newindex, 1);
    lua_settable(L, -3); // metatable.__newindex = setter method

    // Make sure to call destructor when the Actor is GC'd
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, actor_delete);
    lua_settable(L, -3); // meta.__gc = actor_gcNum

    // Prevent metatable from being accessed through Lua
    lua_pushstring(L, "__metatable");
    lua_pushstring(L, "private");
    lua_settable(L, -3);

    // TODO: maybe better to just store functions globally? Like createActor instead
    // Push new table to hold global functions
    //luaL_newlib(L, /*luaL_reg*/);
    lua_newtable(L);
    // TODO: add utility functions
    lua_pushstring(L, "create");
    lua_pushcfunction(L, actor_create);
    lua_settable(L, -3);

    // Assign global function table to global variable
    lua_setglobal(L, "Actor");

    return 0;
}

int Actor::actor_create(lua_State* L)
{
    // Validate arguments
    //lua_Integer spriteID = static_cast<int>(luaL_checkinteger(L, 1));

    // Create Actor userdata and construct Actor object in the allocated memory
    // NOTE: consider using a shared_ptr here instead of the Actor object directly
    Actor* actor = static_cast<Actor*>(lua_newuserdata(L, sizeof(Actor)));
    new(actor) Actor(); // call the constructor on the already allocated block of memory

    //printf("created Actor(%p)\n", actor);

    // Get Actor metatable and assign it to the new Actor
    luaL_getmetatable(L, "Actor");
    lua_setmetatable(L, -2); // -1 is metatable, which gets popped by this call

    // Add empty table as uservalue for storage of runtime variables
    lua_newtable(L);
    lua_setuservalue(L, -2);

    // Create the Actor object
    // TODO: configure Actor based on function arguments
    if (lua_istable(L, 1))
    {
        lua_rawgeti(L, 1, 1); // should return LUA_TNUMBER
        float red = lua_tonumber(L, -1);
        lua_rawgeti(L, 1, 2);
        float green = lua_tonumber(L, -1);
        lua_rawgeti(L, 1, 3);
        float blue = lua_tonumber(L, -1);
        lua_pop(L, 3);
        actor->setGraphics(new GraphicsComponent(red, green, blue));
    }
    else
        actor->setGraphics(new GraphicsComponent());

    // NOTE: do not add Actor to registry until it is hooked into C++ (Canvas)

    return 1;
}

int Actor::actor_delete(lua_State* L)
{
    Actor* actor = static_cast<Actor*>(luaL_checkudata(L, 1, "Actor"));
    //printf("deleting Actor(%p)\n", actor);
    actor->~Actor(); // manually call destructor before Lua calls free()

    return 0;
}

int Actor::actor_index(lua_State* L)
{
    //printf("-- actor_index called\n");

    // Validate Actor userdata
    luaL_checkudata(L, 1, "Actor"); // don't need userdata, just check that it's valid
    //luaL_checktype(L, lua_upvalueindex(1), LUA_TTABLE); // function table

    // First check if the key is in the function table
    lua_pushvalue(L, 2);
    //lua_gettable(L, lua_upvalueindex(1));
    if (lua_rawget(L, lua_upvalueindex(1)) != LUA_TNIL)
    {
        //printf("---- found in function table\n");
        return 1;
    }

    // Get the uservalue from Actor and index it with the 2nd argument
    lua_getuservalue(L, 1);
    lua_pushvalue(L, 2);
    //lua_gettable(L, -2);
    lua_rawget(L, -2); // if type is nil, return it anyway
    /*if (lua_rawget(L, -2) != LUA_TNIL)
        printf("---- found in variable table\n");
    else
        printf("---- variable not found!\n");*/

    return 1;
}

int Actor::actor_newindex(lua_State* L)
{
    //printf("-- actor_newindex called\n");

    // Validate Actor userdata
    luaL_checkudata(L, 1, "Actor"); // don't need userdata, just check that it's valid
    //luaL_checktype(L, lua_upvalueindex(1), LUA_TTABLE); // function table

    // First check if the key is in the function table
    lua_pushvalue(L, 2);
    //lua_gettable(L, lua_upvalueindex(1));
    if (lua_rawget(L, lua_upvalueindex(1)) != LUA_TNIL)
    {
        printf("Attempt to overwrite reserved name \"%s\" on Actor\n", lua_tostring(L, 2));
        return 0;
    }

    // Get the uservalue from Actor and index it with the 2nd argument
    lua_getuservalue(L, 1);
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    // lua_settable(L, -3);
    lua_rawset(L, -3);

    return 0;
}

int Actor::actor_move(lua_State* L)
{
    // Validate Actor userdata
    Actor* actor = static_cast<Actor*>(luaL_checkudata(L, 1, "Actor"));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    actor->moveBy(x, y);

    return 0;
}

int Actor::actor_getPosition(lua_State* L)
{
    // Validate function arguments
    Actor* actor = static_cast<Actor*>(luaL_checkudata(L, 1, "Actor"));

    lua_pushnumber(L, actor->m_transform.getX());
    lua_pushnumber(L, actor->m_transform.getY());
    // TODO: return Z?

    return 2;
}

int Actor::actor_setPosition(lua_State* L)
{
    // Validate function arguments
    Actor* actor = static_cast<Actor*>(luaL_checkudata(L, 1, "Actor"));
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    actor->m_transform.setX(x);
    actor->m_transform.setY(y);

    return 0;
}

int Actor::actor_setScale(lua_State* L)
{
    // Validate function arguments
    Actor* actor = static_cast<Actor*>(luaL_checkudata(L, 1, "Actor"));
    float w = static_cast<float>(luaL_checknumber(L, 2));
    float h = static_cast<float>(luaL_checknumber(L, 3));

    actor->m_transform.setW(w);
    actor->m_transform.setH(h);

    return 0;
}

int Actor::actor_setColor(lua_State* L)
{
    // Validate function arguments
    Actor* actor = static_cast<Actor*>(luaL_checkudata(L, 1, "Actor"));
    luaL_checktype(L, 2, LUA_TTABLE);

    if (lua_istable(L, 2))
    {
        lua_rawgeti(L, 2, 1); // should return LUA_TNUMBER
        float red = lua_tonumber(L, -1);
        lua_rawgeti(L, 2, 2);
        float green = lua_tonumber(L, -1);
        lua_rawgeti(L, 2, 3);
        float blue = lua_tonumber(L, -1);
        lua_pop(L, 3);

        if (GraphicsComponent* graphics = dynamic_cast<GraphicsComponent*>(actor->m_graphics.get()))
            graphics->setColor(red, green, blue);
    }

    return 0;
}

int Actor::actor_setVisible(lua_State* L)
{
    // Validate function arguments
    Actor* actor = static_cast<Actor*>(luaL_checkudata(L, 1, "Actor"));
    luaL_argcheck(L, lua_isboolean(L, 2), 2, "must be boolean (true to pause)\n");

    actor->m_visible = lua_toboolean(L, 2);

    return 0;
}

int Actor::actor_isVisible(lua_State* L)
{
    // Validate function arguments
    Actor* actor = static_cast<Actor*>(luaL_checkudata(L, 1, "Actor"));

    lua_pushboolean(L, actor->m_visible);

    return 1;
}
