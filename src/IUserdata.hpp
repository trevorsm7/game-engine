#pragma once

#include <new>
#include <cassert>
#include <string>
#include <vector>
#include "lua.h"
#include "lauxlib.h"
// TODO limit to lua_State* in this file so we can remove header
//struct lua_State;

class Serializer;
class ObjectRef;

class IUserdata
{
protected:
    static constexpr const char* const CLASS_NAME = "IUserdata";

public:
    virtual ~IUserdata() {}

    void pushUserdata(lua_State* L);
    void pushClone(lua_State* L);

    static int pushMember(lua_State* L, IUserdata* member)
    {
        if (member == nullptr)
            return 0;

        member->pushUserdata(L);
        return 1;
    }

    static IUserdata* testInterface(lua_State* L, int index)
    {
        // TODO add IUserdata* to userdata if necessary
        return reinterpret_cast<IUserdata*>(lua_touserdata(L, index));
    }

    static IUserdata* checkInterface(lua_State* L, int index)
    {
        // TODO add IUserdata* to userdata if necessary
        IUserdata* ptr = reinterpret_cast<IUserdata*>(lua_touserdata(L, index));
        luaL_argcheck(L, ptr, index, "expected IUserdata subtype");
        return ptr;
    }

private:
    template <class, class> friend class TUserdata;
    virtual void* upcastInterface(const void* className) = 0;
    static void* downcastInterface(IUserdata*, const void*) { return nullptr; }

public:
    static void getStringReq(lua_State* L, int index, const char* key, std::string& var)
    {
        lua_pushstring(L, key);
        if (lua_rawget(L, index) != LUA_TSTRING)
            luaL_error(L, "%s required (string)", key);
        var = lua_tostring(L, -1);
        lua_pop(L, 1);
    }

    static void getStringOpt(lua_State* L, int index, const char* key, std::string& var)
    {
        lua_pushstring(L, key);
        if (lua_rawget(L, index) != LUA_TNIL)
        {
            if (lua_type(L, -1) != LUA_TSTRING)
                luaL_error(L, "%s must be string", key);
            var = lua_tostring(L, -1);
        }
        lua_pop(L, 1);
    }

    template <class T>
    static void getValueReq(lua_State* L, int index, const char* key, T& var)
    {
        lua_pushstring(L, key);
        lua_rawget(L, index);
        popT(L, var, -1); // TODO check element type
        lua_pop(L, 1);
    }

    template <class T>
    static void getValueOpt(lua_State* L, int index, const char* key, T& var)
    {
        lua_pushstring(L, key);
        if (lua_rawget(L, index) != LUA_TNIL)
            popT(L, var, -1);
        lua_pop(L, 1);
    }

    template <int N=1>
    static void getListHelper(lua_State* /*L*/) {}

    template <int N=1, class T, class ...As>
    static void getListHelper(lua_State* L, T& arg, As& ...args)
    {
        lua_rawgeti(L, -1, N);
        popT(L, arg, -1); // TODO check element type
        lua_pop(L, 1);
        getListHelper<N+1>(L, args...);
    }

    template <class ...As>
    static void getListReq(lua_State* L, int index, const char* key, As& ...args)
    {
        lua_pushstring(L, key);
        if (lua_rawget(L, index) != LUA_TTABLE)
            luaL_error(L, "%s required (table)", key);

        getListHelper(L, args...);

        lua_pop(L, 1);
    }

    template <class ...As>
    static void getListOpt(lua_State* L, int index, const char* key, As& ...args)
    {
        lua_pushstring(L, key);
        if (lua_rawget(L, index) != LUA_TNIL)
        {
            if (lua_type(L, -1) != LUA_TTABLE)
                luaL_error(L, "%s must be table", key);

            getListHelper(L, args...);
        }

        lua_pop(L, 1);
    }

    template <class T>
    static void getVector(lua_State* L, std::vector<T>& vec)
    {
        for (int i = 0; i < vec.size(); ++i)
        {
            lua_rawgeti(L, -1, i + 1);
            popT(L, vec[i], -1); // TODO check element type
            lua_pop(L, 1);
        }
    }

    template <class T>
    static void getVectorReq(lua_State* L, int index, const char* key, std::vector<T>& vec)
    {
        lua_pushstring(L, key);
        if (lua_rawget(L, index) != LUA_TTABLE)
            luaL_error(L, "%s required (table)", key);

        const int size = vec.size();
        if (lua_rawlen(L, -1) != size)
            luaL_error(L, "%s should be size %d", key, size);

        getVector(L, vec);

        lua_pop(L, 1);
    }

    template <class T>
    static void getVectorOpt(lua_State* L, int index, const char* key, std::vector<T>& vec)
    {
        lua_pushstring(L, key);
        if (lua_rawget(L, index) != LUA_TNIL)
        {
            if (lua_type(L, -1) != LUA_TTABLE)
                luaL_error(L, "%s must be table", key);

            const size_t size = vec.size();
            if (lua_rawlen(L, -1) != size)
                luaL_error(L, "%s should be size %d", key, int(size));

            getVector(L, vec);
        }

        lua_pop(L, 1);
    }

protected:
    void acquireChild(lua_State* L, void* ptr, int index);
    void releaseChild(lua_State* L, void* ptr);

    template <class T>
    void setChild(lua_State* L, int index, T*& child)
    {
        T* ptr = T::checkUserdata(L, index);

        // Do nothing if it's already set
        if (child == ptr)
            return;

        // Clear old child first
        if (child != nullptr)
            releaseChild(L, child);

        // Set the new child
        acquireChild(L, ptr, index);
        child = ptr;
    }

    template <class T>
    void getChildOpt(lua_State* L, int index, const char* key, T*& child)
    {
        lua_pushstring(L, key);
        if (lua_rawget(L, index) != LUA_TNIL)
            setChild(L, -1, child);
        lua_pop(L, 1);
    }

    template <class T>
    void getChildReq(lua_State* L, int index, const char* key, T*& child)
    {
        lua_pushstring(L, key);
        lua_rawget(L, index);
        setChild(L, -1, child);
        lua_pop(L, 1);
    }

    template <class T>
    void copyChild(lua_State* L, T*& dest, T* source)
    {
        if (source)
        {
            source->pushUserdata(L);
            setChild(L, -1, dest);
            lua_pop(L, 1);
        }
    }

    template <class T>
    void cloneChild(lua_State* L, T*& dest, T* source)
    {
        if (source)
        {
            source->pushClone(L);
            setChild(L, -1, dest);
            lua_pop(L, 1);
        }
    }

    bool pcall(lua_State* L, const char* method, int in, int out);

    template <class T> static void pushT(lua_State* L, T arg);
    template <class T> static void popT(lua_State* L, T& arg, int i);

    template <int N=0, class R>
    R pcallT(lua_State* L, const char* method, R ret)
    {
        if (pcall(L, method, N, 1))
        {
            popT(L, ret, -1);
            lua_pop(L, 1);
        }

        return ret;
    }

    template <int N=0, class R, class A, class ...As>
    R pcallT(lua_State* L, const char* method, R ret, A arg, As ...args)
    {
        pushT(L, arg);
        return pcallT<N+1>(L, method, ret, args...);
    }

private:
    // Base cases for TUserdata helper recursion
    static void initInterface(lua_State* L); // TODO rename initHelper or similar?
    static void constructHelper(lua_State* L, IUserdata* ptr, int index);
    static void cloneHelper(lua_State* L, IUserdata* ptr, IUserdata* source, int index);
    static void destroyHelper(lua_State* L, IUserdata* ptr);
    static void serializeHelper(lua_State* L, IUserdata* ptr, Serializer* serializer, ObjectRef* ref);

private:
    static int script_index(lua_State* L);
    static int script_newindex(lua_State* L);
};

template <> inline void IUserdata::pushT(lua_State* L, int8_t arg) {lua_pushinteger(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, uint8_t arg) {lua_pushinteger(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, int16_t arg) {lua_pushinteger(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, uint16_t arg) {lua_pushinteger(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, int32_t arg) {lua_pushinteger(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, uint32_t arg) {lua_pushinteger(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, int64_t arg) {lua_pushinteger(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, uint64_t arg) {lua_pushinteger(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, float arg) {lua_pushnumber(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, double arg) {lua_pushnumber(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, bool arg) {lua_pushboolean(L, arg);}
template <> inline void IUserdata::pushT(lua_State* L, const char* arg) {lua_pushstring(L, arg);}

template <> inline void IUserdata::popT(lua_State* L, int8_t& arg, int i) {arg = int8_t(lua_tointeger(L, i));}
template <> inline void IUserdata::popT(lua_State* L, uint8_t& arg, int i) {arg = uint8_t(lua_tointeger(L, i));}
template <> inline void IUserdata::popT(lua_State* L, int16_t& arg, int i) {arg = int16_t(lua_tointeger(L, i));}
template <> inline void IUserdata::popT(lua_State* L, uint16_t& arg, int i) {arg = uint16_t(lua_tointeger(L, i));}
template <> inline void IUserdata::popT(lua_State* L, int32_t& arg, int i) {arg = int32_t(lua_tointeger(L, i));}
template <> inline void IUserdata::popT(lua_State* L, uint32_t& arg, int i) {arg = uint32_t(lua_tointeger(L, i));}
template <> inline void IUserdata::popT(lua_State* L, int64_t& arg, int i) {arg = int64_t(lua_tointeger(L, i));}
template <> inline void IUserdata::popT(lua_State* L, uint64_t& arg, int i) {arg = uint64_t(lua_tointeger(L, i));}
template <> inline void IUserdata::popT(lua_State* L, float& arg, int i) {arg = float(lua_tonumber(L, i));}
template <> inline void IUserdata::popT(lua_State* L, double& arg, int i) {arg = double(lua_tonumber(L, i));}
template <> inline void IUserdata::popT(lua_State* L, bool& arg, int i) {arg = (lua_toboolean(L, i) == 1);}
template <> inline void IUserdata::popT(lua_State* L, const char*& arg, int i) {arg = lua_tostring(L, i);}

template <class T, class B=IUserdata>
class TUserdata : public B
{
protected:
    // NOTE These must be specialized by derived classes
    //static const char* const CLASS_NAME;
    //static const luaL_Reg METHODS[];

public:
    static void initMetatable(lua_State* L);

    static T* testUserdata(lua_State* L, int index)
    {
        return reinterpret_cast<T*>(luaL_testudata(L, index, T::CLASS_NAME));
    }

    static T* checkUserdata(lua_State* L, int index)
    {
        return reinterpret_cast<T*>(luaL_checkudata(L, index, T::CLASS_NAME));
    }

    static T* testInterface(lua_State* L, int index)
    {
        // TODO add IUserdata* to userdata if necessary
        IUserdata* ptr = reinterpret_cast<IUserdata*>(lua_touserdata(L, index));
        if (!ptr) return nullptr;
        return reinterpret_cast<T*>(ptr->upcastInterface(T::CLASS_NAME));
    }

    static T* checkInterface(lua_State* L, int index)
    {
        // TODO add IUserdata* to userdata if necessary
        IUserdata* ptr = reinterpret_cast<IUserdata*>(lua_touserdata(L, index));
        // TODO generate a "expected "..T::CLASS_NAME.." subtype" literal
        luaL_argcheck(L, ptr, index, T::CLASS_NAME);
        return reinterpret_cast<T*>(ptr->upcastInterface(T::CLASS_NAME));
    }

private:
    template <class, class> friend class TUserdata;

    void* upcastInterface(const void* className) override
    {
        T* ptr = static_cast<T*>(this);
        return downcastInterface(ptr, className);
    }

    static void* downcastInterface(T* ptr, const void* className)//const char* className)
    {
        //if (strcmp(className, T::CLASS_NAME) == 0)
        if (className == T::CLASS_NAME)
            return ptr;

        // Implicitly static_cast up to parent type
        return B::downcastInterface(ptr, className);
    }

private:
    static void initInterface(lua_State* L)
    {
        B::initInterface(L);

        assert(lua_type(L, -2) == LUA_TTABLE);
        lua_pushvalue(L, -2);
        luaL_setfuncs(L, T::METHODS, 0);
        lua_pop(L, 1);
    }

    void construct(lua_State* /*L*/) {} // child inherits no-op
    static void constructHelper(lua_State* L, T* ptr, int index)
    {
        B::constructHelper(L, ptr, index);
        int top = lua_gettop(L);
        ptr->construct(L); // if T::construct is protected, requires friend class
        assert(top == lua_gettop(L));
        (void)top; // unused in release
    }

    void clone(lua_State* /*L*/, T* /*source*/) {} // child inherits no-op
    static void cloneHelper(lua_State* L, T* ptr, T* source, int index)
    {
        B::cloneHelper(L, ptr, source, index);
        int top = lua_gettop(L);
        ptr->clone(L, source);
        assert(top == lua_gettop(L));
        (void)top; // unused in release
    }

    void destroy(lua_State* /*L*/) {} // child inherits no-op
    static void destroyHelper(lua_State* L, T* ptr)
    {
        int top = lua_gettop(L);
        ptr->destroy(L); // if T::destroy is protected, requires friend class
        assert(top == lua_gettop(L));
        (void)top; // unused in release
        B::destroyHelper(L, ptr);
    }

    void serialize(lua_State* /*L*/, Serializer* /*serializer*/, ObjectRef* /*ref*/) {} // child inherits no-op
    static void serializeHelper(lua_State* L, T* ptr, Serializer* serializer, ObjectRef* ref)
    {
        B::serializeHelper(L, ptr, serializer, ref);
        int top = lua_gettop(L);
        ptr->serialize(L, serializer, ref); // if T::serialize is protected, requires friend class
        assert(top == lua_gettop(L));
        (void)top; // unused in release
    }

private:
    static int script_construct(lua_State* L)
    {
        // Validate constructor arguments
        //luaL_checktype(L, 1, LUA_TTABLE); // table/class; unused for now

        // Clone uses index 1, construct uses index 2
        int index = std::min(lua_gettop(L), 2);

        T* source = nullptr;
        if (lua_type(L, index) != LUA_TTABLE)
        {
            source = testUserdata(L, index);
            luaL_argcheck(L, source != nullptr, index, "expected table or class");
        }

        // Create userdata with the full size of the object
        T* ptr = reinterpret_cast<T*>(lua_newuserdata(L, sizeof(T)));

        // Get the metatable for this class
        luaL_getmetatable(L, T::CLASS_NAME);
        assert(lua_type(L, -1) == LUA_TTABLE);
        lua_setmetatable(L, -2);

        new(ptr) T();
        source ? cloneHelper(L, ptr, source, index) : constructHelper(L, ptr, index);

        return 1;
    }

    static int script_destroy(lua_State* L)
    {
        // Validate userdata
        T* ptr = testUserdata(L, 1);
        assert(ptr != nullptr);

        destroyHelper(L, ptr);
        ptr->~T(); // non-virtual

        return 0;
    }

    static int script_serialize(lua_State* L)
    {
        // Validate userdata
        T* ptr = testUserdata(L, 1);
        assert(ptr != nullptr);
        assert(lua_type(L, 2) == LUA_TLIGHTUSERDATA && lua_type(L, 3) == LUA_TLIGHTUSERDATA);
        Serializer* serializer = reinterpret_cast<Serializer*>(lua_touserdata(L, 2));
        ObjectRef* ref = reinterpret_cast<ObjectRef*>(lua_touserdata(L, 3));

        // Call specialized helper function
        serializeHelper(L, ptr, serializer, ref);

        return 0;
    }
};

template <class T, class B>
void TUserdata<T, B>::initMetatable(lua_State* L)
{
    // Create table to populate with methods
    lua_newtable(L);


    // === B::createMetatable(L, CLASS_NAME); ===
    // Push new metatable on the stack
    int rval = luaL_newmetatable(L, T::CLASS_NAME);
    assert(rval == 1); // assert if table already initialized
    (void)rval; // unused in release

    // Prevent metatable from being accessed directly
    lua_pushliteral(L, "__metatable");
    lua_pushstring(L, T::CLASS_NAME); // return name if getmetatable is called
    lua_rawset(L, -3);

    // Push function table to be used with __index and __newindex
    lua_pushliteral(L, "methods");
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);

    // === B::finalizeMetatable(L, CLASS_NAME, script_create, script_delete); ===
    // Make sure to call destructor when the object is GC'd
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, script_destroy);
    lua_rawset(L, -3);

    // Add serialization functionality to all userdata types
    lua_pushliteral(L, "serialize");
    lua_pushcfunction(L, script_serialize);
    lua_rawset(L, -3);

    // Add upcast converter needed for testInterface
    lua_pushliteral(L, "construct");
    lua_pushcfunction(L, script_construct);
    lua_rawset(L, -3);

    initInterface(L);

    // Pop the metatable from the stack
    lua_pop(L, 1);


    // Create read-only class and its metatable
    lua_newuserdata(L, 0);
    lua_newtable(L);

    // Prevent metatable from being accessed directly
    lua_pushliteral(L, "__metatable");
    lua_pushstring(L, T::CLASS_NAME);
    lua_rawset(L, -3);

    // Push methods table as index of class object
    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -4); // methods
    lua_rawset(L, -3);

    // Push constructor as call metamethod
    // NOTE first argument will be the class table
    lua_pushliteral(L, "__call");
    lua_pushcfunction(L, script_construct);
    lua_rawset(L, -3);

    // Set the metatable
    lua_setmetatable(L, -2);

    // Assign class as a global variable
    lua_pushstring(L, T::CLASS_NAME);
    lua_insert(L, -2); // swap name and class
    lua_rawset(L, -4); // globals TODO replace with index passed into function

    // Pop the methods table
    lua_pop(L, 1);
}
