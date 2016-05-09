#ifndef __TUSERDATA_H__
#define __TUSERDATA_H__

#include <new>
#include <cassert>
#include "lua.hpp"

template <class T>
class TUserdata
{
    int m_refCount;
public:
    TUserdata(): m_refCount(0) {}

    static void initMetatable(lua_State* L);
    static T* checkUserdata(lua_State* L, int index)
        {return reinterpret_cast<T*>(luaL_checkudata(L, index, T::METATABLE));}

    bool pushUserdata(lua_State* L);
    // NOTE should be protected? Wrap in unique_ptr-like class?
    void refAdded(lua_State* L, int index);
    void refRemoved(lua_State* L);

protected:
    bool pcall(lua_State* L, const char* method, int in, int out);

private:
    static int script_index(lua_State* L);
    static int script_newindex(lua_State* L);
    static int script_create(lua_State* L);
    static int script_delete(lua_State* L);
};

template <class T>
void TUserdata<T>::initMetatable(lua_State* L)
{
    // TODO the push string/table functions are unsafe if they fail (out of memory error)
    int top = lua_gettop(L);

    // Push new metatable on the stack
    luaL_newmetatable(L, T::METATABLE);

    // Push function table to be used with __index and __newindex
    lua_pushliteral(L, "methods");
    luaL_newlib(L, T::METHODS);
    lua_rawset(L, -3);

    // Member reads come either from method table or uservalue table
    lua_pushliteral(L, "__index");
    lua_pushcfunction(L, script_index);
    lua_rawset(L, -3);

    // Member writes go to uservalue table
    lua_pushliteral(L, "__newindex");
    lua_pushcfunction(L, script_newindex);
    lua_rawset(L, -3);

    // Make sure to call destructor when the object is GC'd
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, script_delete);
    lua_rawset(L, -3);

    // Prevent metatable from being accessed directly
    lua_pushliteral(L, "__metatable");
    lua_pushstring(L, T::METATABLE); // return name if getmetatable is called
    lua_rawset(L, -3);

    // Pop the metatable from the stack
    lua_pop(L, 1);

    // Push constructor as global function with class name
    lua_pushcfunction(L, script_create);
    lua_setglobal(L, T::METATABLE);

    assert(top == lua_gettop(L));
}

template <class T>
bool TUserdata<T>::pushUserdata(lua_State* L)
{
    lua_pushlightuserdata(L, this);
    lua_rawget(L, LUA_REGISTRYINDEX);
    // NOTE this pointer will not always == the base address of the userdata!
    // TODO could store the TUserdata<T> this pointer as light udata in uservalue?
    assert(luaL_testudata(L, -1, T::METATABLE));
    //assert(lua_touserdata(L, -1) == this);
    /*if (lua_touserdata(L, -1) != this)
    {
        fprintf(stderr, "Userdata pointer in registry doesn't match\n");
        return false;
    }*/

    return true;
}

template <class T>
void TUserdata<T>::refAdded(lua_State* L, int index)
{
    // TODO use get rid of ref counting? make a container to manage these?
    // Add userdata to the registry while it is ref'd by engine
    if (m_refCount++ == 0)
    {
        lua_pushlightuserdata(L, this);
        // TODO assert that full userdata pointer matches this?
        lua_pushvalue(L, (index < 0) ? index - 1 : index); // adjust relative offset
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
}

template <class T>
void TUserdata<T>::refRemoved(lua_State* L)
{
    // TODO use get rid of ref counting? make a container to manage these?
    // Remove from registry if reference count drops to zero
    if (--m_refCount == 0)
    {
        //printf("removing %s(%p) from registry\n", T::METATABLE, this);
        lua_pushlightuserdata(L, this);
        lua_pushnil(L);
        lua_rawset(L, LUA_REGISTRYINDEX);
    }
}

template <class T>
bool TUserdata<T>::pcall(lua_State* L, const char* method, int in, int out)
{
    if (!pushUserdata(L))
    {
        // TODO can probably just make this an assert; might not even need it if pushUserdata does the same
        fprintf(stderr, "Attempt to call %s:%s with invalid userdata (%p)\n", T::METATABLE, method, this);
        lua_pop(L, in + 1);
        return false;
    }

    // Push uservalue on stack ([in] udata uvalue)
    lua_getuservalue(L, -1);
    assert(lua_type(L, -1) == LUA_TTABLE);

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

template <class T>
int TUserdata<T>::script_index(lua_State* L)
{
    // Validate userdata
    //assert(luaL_testudata(L, 1, T::METATABLE));
    checkUserdata(L, 1);

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

template <class T>
int TUserdata<T>::script_newindex(lua_State* L)
{
    // Validate userdata
    //assert(luaL_testudata(L, 1, T::METATABLE));
    checkUserdata(L, 1);

    // First check if the key is in the function table
    if (luaL_getmetafield(L, 1, "methods") == LUA_TTABLE)
    {
        lua_pushvalue(L, 2);
        if (lua_rawget(L, -2) != LUA_TNIL)
            luaL_error(L, "field %s is read-only!", lua_tostring(L, 2));
    }

    // Get the uservalue from Actor and index it with the 2nd argument
    if (lua_getuservalue(L, 1) == LUA_TTABLE)
    {
        lua_pushvalue(L, 2);
        lua_pushvalue(L, 3);
        lua_rawset(L, -3);
    }
    else
        luaL_error(L, "class %s is read-only!", T::METATABLE);

    return 0;
}

template <class T>
int TUserdata<T>::script_create(lua_State* L)
{
    // Check metatable first
    luaL_getmetatable(L, T::METATABLE);
    assert(lua_type(L, -1) == LUA_TTABLE);

    // Create userdata with the full size of the object
    T* ptr = reinterpret_cast<T*>(lua_newuserdata(L, sizeof(T)));

    // Rearrange userdata and metatable, then set metatable to userdata
    lua_insert(L, -2);
    lua_setmetatable(L, -2);

    // Lastly, initialize the object with arguments passed from Lua
    new(ptr) T();
    int top = lua_gettop(L);
    ptr->construct(L);
    assert(top == lua_gettop(L));

    return 1;
}

template <class T>
int TUserdata<T>::script_delete(lua_State* L)
{
    // Manually call destructor before Lua frees memory
    T* ptr = checkUserdata(L, 1);
    ptr->destroy(L);
    ptr->~T();

    return 0;
}

#endif
