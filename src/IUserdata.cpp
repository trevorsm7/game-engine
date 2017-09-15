#include "IUserdata.hpp"
#include "Serializer.hpp"
#include "Scene.hpp"

void IUserdata::pushUserdata(lua_State* L)
{
    lua_pushstring(L, Scene::WEAK_REFS);
    lua_rawget(L, LUA_REGISTRYINDEX);
    assert(lua_type(L, -1) == LUA_TTABLE);

    // NOTE IUserdata::this != base address; only use as lookup pointer
    lua_pushlightuserdata(L, this);
    lua_rawget(L, -2);
    lua_remove(L, -2);

    assert(testInterface(L, -1) == this); // upcast to IUserdata
}

void IUserdata::pushClone(lua_State* L)
{
    pushUserdata(L);

    // Get userdata clone function
    luaL_getmetafield(L, -1, "construct");
    assert(lua_type(L, -1) == LUA_TFUNCTION);

    // Swap the userdata and the function
    lua_rotate(L, -2, -1);

    // Do a regular call; pops function and arguments
    // TODO use pcall? see script_clone behavior
    lua_call(L, 1, 1);
    assert(lua_touserdata(L, -1) != nullptr);
}

void IUserdata::acquireChild(lua_State* L, void* ptr, int index)
{
    // Get the child ref table
    assert(ptr != nullptr);
    lua_pushlightuserdata(L, this);
    int type = lua_rawget(L, LUA_REGISTRYINDEX);
    assert(type == LUA_TNIL || type == LUA_TTABLE);

    // Create the table if it doesn't exist
    if (type == LUA_TNIL)
    {
        lua_pop(L, 1);
        lua_createtable(L, 0, 1);

        lua_pushlightuserdata(L, this);
        lua_pushvalue(L, -2);
        lua_rawset(L, LUA_REGISTRYINDEX);
    }

    // Add the child ref
    lua_pushlightuserdata(L, ptr);
    lua_pushvalue(L, index < 0 ? index - 2 : index);
    assert(lua_type(L, -1) == LUA_TUSERDATA);
    lua_rawset(L, -3);

    lua_pop(L, 1);
}

void IUserdata::releaseChild(lua_State* L, void* ptr)
{
    // Get the child ref table
    assert(ptr != nullptr);
    lua_pushlightuserdata(L, this);
    lua_rawget(L, LUA_REGISTRYINDEX);
    assert(lua_type(L, -1) == LUA_TTABLE);

    // Delete the child ref
    lua_pushlightuserdata(L, ptr);
    lua_pushnil(L);
    lua_rawset(L, -3);

    lua_pop(L, 1);
}

bool IUserdata::pcall(lua_State* L, const char* method, int in, int out)
{
    pushUserdata(L);

    // Push uservalue on stack ([in] udata uvalue)
    if (lua_getuservalue(L, -1) != LUA_TTABLE)
    {
        // NOTE this is not an error so don't spam error messages about it!
        lua_pop(L, in + 2);
        return false;
    }

    // Push function on stack ([in] udata uvalue function)
    lua_pushstring(L, method); // TODO this is unsafe if it fails (out of memory error)
    if (lua_rawget(L, -2) != LUA_TFUNCTION)
    {
        // NOTE this is not an error so don't spam error messages about it!
        lua_pop(L, in + 3);
        return false;
    }

    // Reorder stack (function udata [in])
    lua_insert(L, -(in + 3)); // insert function before args
    lua_pop(L, 1); // remove the uservalue from the stack
    lua_insert(L, -(in + 1)); // insert udata before args

    // Set 30 ms watchdog to protect against malformed or excessively slow code
    Scene* scene = Scene::checkScene(L);
    scene->setWatchdog(30);

    // Do a protected call; pops function, udata, and args
    int rval = lua_pcall(L, in + 1, out, 0);
    scene->clearWatchdog();

    if (rval != 0)
    {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // remove the error string from the stack
        return false;
    }

    return true;
}

void IUserdata::initInterface(lua_State* L)
{
    //setMethods(L, METHODS);//T::METHODS);

    // Member reads come either from method table or uservalue table
    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, script_index);
    lua_rawset(L, -3);

    // Member writes go to uservalue table
    lua_pushliteral(L, "__newindex");
    lua_pushcfunction(L, script_newindex);
    lua_rawset(L, -3);
}

static inline void add_weak_ref(lua_State* L, IUserdata* ptr)
{
    // Add weak ref to userdata lookup table
    lua_pushstring(L, Scene::WEAK_REFS);
    lua_rawget(L, LUA_REGISTRYINDEX);
    assert(lua_type(L, -1) == LUA_TTABLE);
    // NOTE IUserdata::this != base address; only use as lookup pointer
    lua_pushlightuserdata(L, ptr);
    lua_pushvalue(L, -3); // the userdata
    lua_rawset(L, -3);
    lua_pop(L, 1);
}

static inline void copy_uservalues(lua_State* L)
{
    // Create a new table for shallow copy
    lua_newtable(L);

    // Iterate over table, copying key/value pairs into the uservalue table
    lua_pushnil(L);
    while (lua_next(L, -3))
    {
        // Duplicate key, set key/value, leave key on stack for next iteration of lua_next
        lua_pushvalue(L, -2);
        lua_insert(L, -2);
        lua_rawset(L, -4);
    }

    // Set table as uservalue of userdata
    // NOTE userdata should have been on top before helper called
    assert(lua_type(L, -3) == LUA_TUSERDATA);
    lua_setuservalue(L, -3);
}

void IUserdata::constructHelper(lua_State* L, IUserdata* ptr, int index)
{
    add_weak_ref(L, ptr);

    // Copy the member table if specified
    lua_pushliteral(L, "members");
    if (lua_rawget(L, index) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        copy_uservalues(L);
    }
    lua_pop(L, 1);
}

void IUserdata::cloneHelper(lua_State* L, IUserdata* ptr, IUserdata* /*source*/, int index)
{
    add_weak_ref(L, ptr);

    if (lua_getuservalue(L, index) != LUA_TNIL)
    {
        assert(lua_type(L, -1) == LUA_TTABLE);
        copy_uservalues(L);
    }
    lua_pop(L, 1);
}

void IUserdata::destroyHelper(lua_State* L, IUserdata* ptr)
{
    // Delete child ref table
    lua_pushlightuserdata(L, ptr);
    lua_pushnil(L);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

void IUserdata::serializeHelper(lua_State* L, IUserdata* /*ptr*/, Serializer* serializer, ObjectRef* ref)
{
    // Serialize members subtable
    if (lua_getuservalue(L, 1) == LUA_TTABLE)
        serializer->serializeSubtable(ref, "members", L, -1);
    lua_pop(L, 1);
}

int IUserdata::script_index(lua_State* L)
{
    // Validate userdata
    assert(lua_type(L, 1) == LUA_TUSERDATA);

    // Check if key is in the members table (uservalue)
    if (lua_getuservalue(L, 1) == LUA_TTABLE)
    {
        lua_pushvalue(L, 2);
        if (lua_rawget(L, -2) != LUA_TNIL)
            return 1;
    }

    // Check if key is in the methods table (metafield)
    if (luaL_getmetafield(L, 1, "methods") == LUA_TTABLE)
    {
        lua_pushvalue(L, 2);
        if (lua_rawget(L, -2) != LUA_TNIL)
            return 1;
    }

    return 0;
}

int IUserdata::script_newindex(lua_State* L)
{
    // Validate userdata
    assert(lua_type(L, 1) == LUA_TUSERDATA);

    // Get the uservalue table; initialize if it doesn't exist
    if (lua_getuservalue(L, 1) != LUA_TTABLE)
    {
        // Remove the nil from the stack
        assert(lua_type(L, -1) == LUA_TNIL);
        lua_pop(L, 1);

        // Create a new uservalue table; leave a copy on the stack
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setuservalue(L, 1);
    }

    // Index the uservalue table with [key] = value
    lua_pushvalue(L, 2);
    lua_pushvalue(L, 3);
    lua_rawset(L, -3);

    return 0;
}
