#ifndef __ICOLLIDER_H__
#define __ICOLLIDER_H__

#include "Aabb.h"

#include <cstdint>

class Actor;

class ICollider
{
    uint32_t m_colliderGroup;
    uint32_t m_colliderMask;
    bool m_collidable;

public:
    ICollider(uint32_t group, uint32_t mask): m_colliderGroup(group), m_colliderMask(mask), m_collidable(true) {}
    virtual ~ICollider() {}

    // TODO: when would we need to update collider?
    virtual void update(float delta) = 0;

    virtual bool testCollision(float x, float y) const = 0;
    virtual bool testCollision(const Aabb& aabb) const = 0;
    virtual bool testCollision(float deltaX, float deltaY, const ICollider* other) const = 0;

    virtual bool getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const = 0;
    virtual bool getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const = 0;

    bool isCollidable() const {return m_collidable;}
    bool isCollidableWith(const ICollider* other) const {return m_collidable && other && other->m_collidable && isMasked(other) && other->isMasked(this);}
    void setCollidable(bool collidable) {m_collidable = collidable;}

private:
    bool isMasked(const ICollider* other) const {return m_colliderGroup & other->m_colliderMask;}
};

#endif
