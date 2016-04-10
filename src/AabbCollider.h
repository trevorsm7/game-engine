#ifndef __AABBCOLLIDER_H__
#define __AABBCOLLIDER_H__

#include "ICollider.h"
#include "Aabb.h"

class Actor;

class AabbCollider : public ICollider
{
    Actor* m_actor;

public:
    AabbCollider(Actor* actor, uint32_t group, uint32_t mask): ICollider(group, mask), m_actor(actor) {}
    ~AabbCollider() override {}

    void update(float delta) override {}

    bool testCollision(float x, float y) const override;
    bool testCollision(const Aabb& aabb) const override;
    bool testCollision(float deltaX, float deltaY, const ICollider* other) const override;

    bool getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const override;
    bool getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const override;
};

#endif
