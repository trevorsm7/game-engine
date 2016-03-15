#ifndef __AABBCOLLIDER_H__
#define __AABBCOLLIDER_H__

#include "ICollider.h"
#include "Actor.h"

class AabbCollider : public ICollider
{
    Actor* m_actor;

public:
    AabbCollider(Actor* actor): m_actor(actor) {}
    ~AabbCollider() override {}

    void update(float delta) override {}

    bool testCollision(float x, float y) const override
    {
        if (!isCollidable() || !m_actor)
            return false;

        const Aabb self = m_actor->getTransform().getAabb();
        return self.isContaining(x, y);
    }

    bool testCollision(const Aabb& aabb) const override
    {
        if (!isCollidable() || !m_actor)
            return false;

        const Aabb self = m_actor->getTransform().getAabb();
        return self.isOverlapped(aabb);
    }

    bool testCollision(const ICollider* other) const override
    {
        if (!isCollidable() || !m_actor || !other)
            return false;

        Aabb bounds = m_actor->getTransform().getAabb();
        return other->testCollision(bounds);
    }
};

#endif
