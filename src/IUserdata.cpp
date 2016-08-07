#include "IUserdata.hpp"

void IUserdata::pushUserdata(lua_State* L)
{
    // NOTE IUserdata::this != base address; only use as a registry key
    lua_pushlightuserdata(L, this);
    lua_rawget(L, LUA_REGISTRYINDEX);
    //assert(lua_type(L, -1) == LUA_TUSERDATA);
    assert(testInterface(L, -1) == this); // upcast to IUserdata
}

void IUserdata::refAdded(lua_State* L, int index)
{
    //assert(lua_type(L, index) == LUA_TUSERDATA);
    assert(testInterface(L, index) == this); // upcast to IUserdata
    // TODO use get rid of ref counting? make a container to manage these?
    // Add userdata to the registry while it is ref'd by engine
    if (m_refCount++ == 0)
    {
        lua_pushlightuserdata(L, this);
        lua_pushvalue(L, (index < 0) ? index - 1 : index); // adjust relative offset
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
}

void IUserdata::refRemoved(lua_State* L)
{
    // TODO use get rid of ref counting? make a container to manage these?
    // Remove from registry if reference count drops to zero
    if (--m_refCount == 0)
    {
        lua_pushlightuserdata(L, this);
        lua_pushnil(L);
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
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

    // Do a protected call; pops function, udata, and args
    if (lua_pcall(L, in + 1, out, 0) != 0)
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

void IUserdata::constructHelper(lua_State* L, IUserdata* ptr)
{
    // Copy the member table if specified
    lua_pushliteral(L, "members");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);

        // Create a new table for shallow copy
        lua_newtable(L);

        // Iterate over table, copying key/value pairs into the uservalue table
        lua_pushnil(L);
        while (lua_next(L, -3))
        {
            luaL_argcheck(L, lua_type(L, -2) == LUA_TSTRING, 1, "member tables keys must be strings");

            // Duplicate key, set key/value, leave key on stack for next iteration of lua_next
            lua_pushvalue(L, -2);
            lua_insert(L, -2);
            lua_rawset(L, -4);
        }

        // Set table as uservalue of userdata
        // NOTE is it expected that userdata be at -1 at the start
        assert(lua_type(L, -3) == LUA_TUSERDATA);
        lua_setuservalue(L, -3);
    }
    lua_pop(L, 1);
}

void IUserdata::serializeHelper(lua_State* L, IUserdata* ptr, Serializer* serializer, ObjectRef* ref)
{
    // Serialize members subtable
    if (lua_getuservalue(L, 1) == LUA_TTABLE)
        serializer->serializeFromTable(ref, "members", L, -1);
    lua_pop(L, 1);
}

void* IUserdata::testInterfaceBase(lua_State* L, int index, void* className)
{
    // Get userdata upcast function
    luaL_getmetafield(L, index, "upcast");
    if (lua_type(L, -1) != LUA_TFUNCTION)
        return nullptr;

    // Push the userdata and pointer as arguments
    const int adjIndex = (index < 0) ? index - 1 : index;
    lua_pushvalue(L, adjIndex);
    //lua_pushstring(L, className);
    lua_pushlightuserdata(L, className);

    // Do a regular call; pops function and arguments
    // TODO use pcall? see script_upcast behavior
    lua_call(L, 2, 1);

    // Retrieve the upcast pointer (safe to reinterpret_cast)
    void* ptr = lua_touserdata(L, -1);
    lua_pop(L, 1);

    return ptr;
}

int IUserdata::script_index(lua_State* L)
{
    // Validate userdata
    assert(lua_type(L, 1) == LUA_TUSERDATA);
    //assert(luaL_testudata(L, 1, T::METATABLE));
    //checkUserdata(L, 1);

    // First check if the key is in the function table
    if (luaL_getmetafield(L, 1, "methods") == LUA_TTABLE)
    {
        lua_pushvalue(L, 2);
        if (lua_rawget(L, -2) != LUA_TNIL)
            return 1;
    }

    // Get the uservalue from Actor and index it with the 2nd argument
    if (lua_getuservalue(L, 1) == LUA_TTABLE)
    {
        lua_pushvalue(L, 2);
        lua_rawget(L, -2); // if type is nil, return it anyway
        return 1;
    }

    lua_pushnil(L);
    return 1;
}

int IUserdata::script_newindex(lua_State* L)
{
    // Validate userdata
    assert(lua_type(L, 1) == LUA_TUSERDATA);
    //assert(luaL_testudata(L, 1, T::METATABLE));
    //checkUserdata(L, 1);

    // First check if the key is in the function table
    if (luaL_getmetafield(L, 1, "methods") == LUA_TTABLE)
    {
        lua_pushvalue(L, 2);
        if (lua_rawget(L, -2) != LUA_TNIL)
            luaL_error(L, "field %s is read-only!", lua_tostring(L, 2));
    }

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
