#ifndef __IUSERDATA_HPP__
#define __IUSERDATA_HPP__

#include "Serializer.h"

#include <new>
#include <cassert>
#include "lua.hpp"
// TODO limit to lua_State* in this file so we can remove header
//struct lua_State;

// TODO limit to Serializer ptr in this file so we can remove header
//class Serializer;
//class ObjectRef;

class IUserdata
{
    int m_refCount;

public:
    IUserdata(): m_refCount(0) {}

public:
    void pushUserdata(lua_State* L);
    // NOTE should be protected? Wrap in unique_ptr-like class?
    void refAdded(lua_State* L, int index);
    void refRemoved(lua_State* L);

protected:
    bool pcall(lua_State* L, const char* method, int in, int out);

    // Base cases for TUserdata helper recursion
    static void initInterface(lua_State* L); // TODO rename initHelper or similar?
    static void constructHelper(lua_State* L, IUserdata* ptr);
    static void destroyHelper(lua_State* L, IUserdata* ptr) {}
    static void serializeHelper(lua_State* L, IUserdata* ptr, Serializer* serializer, ObjectRef* ref);

private:
    static int script_index(lua_State* L);
    static int script_newindex(lua_State* L);
};

template <class T, class B=IUserdata>
class TUserdata : public B
{
protected:
    // NOTE These must be specialized by derived classes
    //static const char* const CLASS_NAME;
    //static const luaL_Reg METHODS[];

public:
    // TODO merge checkUserdata and checkInterface
    // TODO add testUserdata method as well
    static T* checkUserdata(lua_State* L, int index)
    {
        return reinterpret_cast<T*>(luaL_checkudata(L, index, T::CLASS_NAME));
    }

    static T* checkInterface(lua_State* L, int index)
    {
        bool valid = false;

        // NOTE getmetafield pushes nothing when LUA_TNIL is returned
        if (luaL_getmetafield(L, index, T::CLASS_NAME) != LUA_TNIL)
        {
            if (lua_type(L, -1) == LUA_TBOOLEAN && lua_toboolean(L, -1))
                valid = true;

            lua_pop(L, 1);
        }

        if (!valid)
            luaL_error(L, "expected userdata with interface %s", T::CLASS_NAME);

        // TODO should cast to IUserdata, then dynamic_cast down?
        // TODO should reinterpret_cast to the real type, then static_cast up?
        return reinterpret_cast<T*>(lua_touserdata(L, index));
    }

    static void initMetatable(lua_State* L);

protected:
    static void initInterface(lua_State* L)
    {
        B::initInterface(L);

        //B::setInterface(L, CLASS_NAME);
        // Tag metatable as an instance of IUserdata
        lua_pushstring(L, T::CLASS_NAME);
        lua_pushboolean(L, true);
        lua_rawset(L, -3);

        //B::setMethods(L, METHODS);//T::METHODS);
        lua_pushliteral(L, "methods");
        lua_rawget(L, -2);
        assert(lua_type(L, -1) == LUA_TTABLE);
        luaL_setfuncs(L, T::METHODS, 0);
        lua_pop(L, 1);
    }

    void construct(lua_State* L) {} // child inherits no-op
    static void constructHelper(lua_State* L, T* ptr)
    {
        B::constructHelper(L, ptr);
        int top = lua_gettop(L);
        ptr->construct(L); // if T::construct is protected, requires friend class
        assert(top == lua_gettop(L));
    }

    void destroy(lua_State* L) {} // child inherits no-op
    static void destroyHelper(lua_State* L, T* ptr)
    {
        ptr->destroy(L); // if T::destroy is protected, requires friend class
        B::destroyHelper(L, ptr);
    }

    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref) {} // child inherits no-op
    static void serializeHelper(lua_State* L, T* ptr, Serializer* serializer, ObjectRef* ref)
    {
        B::serializeHelper(L, ptr, serializer, ref);
        int top = lua_gettop(L);
        ptr->serialize(L, serializer, ref); // if T::serialize is protected, requires friend class
        assert(top == lua_gettop(L));
    }

private:
    static int script_create(lua_State* L)
    {
        // Validate constructor arguments
        luaL_checktype(L, 1, LUA_TTABLE);

        // Create userdata with the full size of the object
        T* ptr = reinterpret_cast<T*>(lua_newuserdata(L, sizeof(T)));

        // Get the metatable for this class
        luaL_getmetatable(L, T::CLASS_NAME);
        assert(lua_type(L, -1) == LUA_TTABLE);
        lua_setmetatable(L, -2);

        new(ptr) T();
        constructHelper(L, ptr);

        return 1;
    }

    static int script_destroy(lua_State* L)
    {
        T* ptr = checkUserdata(L, 1); // TODO assert instead?

        destroyHelper(L, ptr);
        ptr->~T(); // non-virtual

        return 0;
    }

    static int script_serialize(lua_State* L)
    {
        // Validate userdata
        T* ptr = checkUserdata(L, 1); // TODO assert instead?
        Serializer* serializer = Serializer::checkSerializer(L, 2);

        // Get the object ref and update the constructor name
        ObjectRef* ref = serializer->getObjectRef(ptr);
        assert(ref != nullptr);
        ref->setConstructor(T::CLASS_NAME);

        // Call specialized helper function
        serializeHelper(L, ptr, serializer, ref);

        return 0;
    }
};

template <class T, class B>
void TUserdata<T, B>::initMetatable(lua_State* L)
{
    // TODO the push string/table functions are unsafe if they fail (out of memory error)
    // TODO assert if init is called more than once?

    // === B::createMetatable(L, CLASS_NAME); ===
    // Push new metatable on the stack
    luaL_newmetatable(L, T::CLASS_NAME);

    // Prevent metatable from being accessed directly
    lua_pushliteral(L, "__metatable");
    lua_pushstring(L, T::CLASS_NAME); // return name if getmetatable is called
    lua_rawset(L, -3);


    // TODO replace with setMethods, move newlib/newtable elsewhere
    // Push function table to be used with __index and __newindex
    lua_pushliteral(L, "methods");
    //luaL_newlib(L, METHODS);//T::METHODS);
    lua_newtable(L);
    lua_rawset(L, -3);

    initInterface(L);
    //B::initInterface(L);
    //B::setMethods(L, METHODS);//T::METHODS);


    // === B::finalizeMetatable(L, CLASS_NAME, script_create, script_delete); ===
    // Make sure to call destructor when the object is GC'd
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, script_destroy);
    lua_rawset(L, -3);

    // Add serialization functionality to all userdata types
    lua_pushliteral(L, "serialize");
    lua_pushcfunction(L, script_serialize);
    lua_rawset(L, -3);

    // Pop the metatable from the stack
    lua_pop(L, 1);

    // Push constructor as global function with class name
    lua_pushcfunction(L, script_create);
    lua_setglobal(L, T::CLASS_NAME);
}

#endif
