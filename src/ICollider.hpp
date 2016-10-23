#pragma once

#include "IUserdata.hpp"
#include "Aabb.hpp"

#include <cstdint>
#include <cassert>

class Actor;

class ICollider : public TUserdata<ICollider>
{
public:
    Actor* m_actor;

private:
    uint32_t m_colliderGroup;
    uint32_t m_colliderMask;
    bool m_collidable;

protected:
    ICollider(): m_actor(nullptr), m_colliderGroup(1), m_colliderMask(0xFFFFFFFF), m_collidable(true) {}

public:
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

private:
    friend class TUserdata<ICollider>;
    void construct(lua_State* L);
    void clone(lua_State* L, ICollider* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_setCollidable(lua_State* L);

    static constexpr const char* const CLASS_NAME = "ICollider";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"setCollidable", script_setCollidable},
        {nullptr, nullptr}
    };
};
