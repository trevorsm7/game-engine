#ifndef __ICOLLIDER_H__
#define __ICOLLIDER_H__

#include "Aabb.h"

class Actor;

class ICollider
{
    bool m_collidable;

public:
    ICollider(): m_collidable(true) {}
    virtual ~ICollider() {}

    // TODO: when would we need to update collider?
    virtual void update(float delta) = 0;

    virtual bool testCollision(float x, float y) const = 0;
    virtual bool testCollision(const Aabb& aabb) const = 0;
    virtual bool testCollision(float deltaX, float deltaY, const ICollider* other) const = 0;

    virtual bool getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const = 0;
    virtual bool getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const = 0;

    // TODO remove virtual if we don't need to override
    virtual bool isCollidable() const {return m_collidable;}
    virtual void setCollidable(bool collidable) {m_collidable = collidable;}
};

#endif
