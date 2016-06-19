#ifndef __ICOLLIDER_H__
#define __ICOLLIDER_H__

#include "TUserdata.h"
#include "Aabb.h"

#include <cstdint>
#include <cassert>

class Actor;

class ICollider
{
public:
    Actor* m_actor;
private:
    uint32_t m_colliderGroup;
    uint32_t m_colliderMask;
    bool m_collidable;

public:
    ICollider(): m_actor(nullptr), m_colliderGroup(1), m_colliderMask(0xFFFFFFFF), m_collidable(true) {}
    virtual ~ICollider() {}

    // TODO: when would we need to update collider?
    virtual void update(float delta) = 0;

    virtual bool testCollision(float x, float y) const = 0;
    virtual bool testCollision(const Aabb& aabb) const = 0;
    virtual bool testCollision(float deltaX, float deltaY, const ICollider* other) const = 0;

    virtual bool getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const = 0;
    virtual bool getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const = 0;

    bool isCollidable() const {return m_collidable;}
    bool isCollidableWith(const ICollider* other) const {assert(other); return isCollidable() && other->isCollidable() && isMasked(other) && other->isMasked(this);}
    void setCollidable(bool collidable) {m_collidable = collidable;}

    void setGroup(uint32_t group) {m_colliderGroup = group;}
    void setMask(uint32_t mask) {m_colliderMask = mask;}
    bool isMasked(const ICollider* other) const {assert(other); return m_colliderGroup & other->m_colliderMask;}

    static ICollider* testUserdata(lua_State* L, int index)
    {
        ICollider* collider = nullptr;

        // NOTE getmetafield pushes nothing when LUA_TNIL is returned
        if (luaL_getmetafield(L, index, INTERFACE) != LUA_TNIL)
        {
            lua_pop(L, 1);
            collider = reinterpret_cast<ICollider*>(lua_touserdata(L, index));
        }

        return collider;
    }

    static ICollider* checkUserdata(lua_State* L, int index)
    {
        ICollider* collider = testUserdata(L, index);

        if (!collider)
            luaL_error(L, "expected userdata with interface %s", INTERFACE);

        return collider;
    }

    // Virtual functions to be implemented by TGraphics template
    virtual bool pushUserdata(lua_State* L) = 0;
    virtual void refAdded(lua_State* L, int index) = 0;
    virtual void refRemoved(lua_State* L) = 0;

protected:
    virtual bool pcall(lua_State* L, const char* method, int in, int out) = 0;

    static constexpr const char* const INTERFACE = "ICollider";
};

template <class T>
class TCollider : public ICollider, public TUserdata<T>
{
public:
    static void initMetatable(lua_State* L);

    bool pushUserdata(lua_State* L) override {return TUserdata<T>::pushUserdata(L);}
    void refAdded(lua_State* L, int index) override {TUserdata<T>::refAdded(L, index);}
    void refRemoved(lua_State* L) override {TUserdata<T>::refRemoved(L);}

protected:
    bool pcall(lua_State* L, const char* method, int in, int out) override
        {return TUserdata<T>::pcall(L, method, in, out);}

    void construct(lua_State* L);
    //void destroy(lua_State* L) {}

    static int script_setCollidable(lua_State* L);
    static const luaL_Reg METHODS[];
};

template <class T>
const luaL_Reg TCollider<T>::METHODS[] =
{
    {"setCollidable", TCollider<T>::script_setCollidable},
    {nullptr, nullptr}
};

template <class T>
void TCollider<T>::initMetatable(lua_State* L)
{
    int top = lua_gettop(L);
    TUserdata<T>::initMetatable(L);

    // Push the metatable back on the stack
    luaL_getmetatable(L, T::METATABLE);
    assert(lua_type(L, -1) == LUA_TTABLE);

    // Push the interface name as a key
    lua_pushstring(L, ICollider::INTERFACE);
    lua_pushboolean(L, true);
    lua_rawset(L, -3);

    // Push interface methods
    // NOTE compiler should trim this if sublcass doesn't define T::METHODS
    if (T::METHODS != TCollider<T>::METHODS)
    {
        lua_pushliteral(L, "methods");
        lua_rawget(L, -2);
        assert(lua_type(L, -1) == LUA_TTABLE);
        luaL_setfuncs(L, TCollider<T>::METHODS, 0);
        lua_pop(L, 1);
    }

    // Pop the metatable and method table from the stack
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

template <class T>
void TCollider<T>::construct(lua_State* L)
{
    lua_pushliteral(L, "group");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TNUMBER);
        setGroup(lua_tointeger(L, -1));
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "mask");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TNUMBER);
        setMask(lua_tointeger(L, -1));
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "collidable");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TBOOLEAN);
        setCollidable(lua_toboolean(L, -1));
    }
    lua_pop(L, 1);
}

template <class T>
int TCollider<T>::script_setCollidable(lua_State* L)
{
    // Validate function arguments
    T* self = TUserdata<T>::checkUserdata(L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    self->setCollidable(lua_toboolean(L, 2));

    // Return self userdata
    lua_pushvalue(L, 1);
    return 1;
}

#endif
