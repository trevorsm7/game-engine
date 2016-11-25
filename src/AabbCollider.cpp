#include "AabbCollider.hpp"
#include "Actor.hpp"

#include <cassert>

const luaL_Reg AabbCollider::METHODS[];

bool AabbCollider::testCollision(float x, float y) const
{
    assert(m_actor != nullptr);

    if (!isCollidable())
        return false;

    const Aabb self = m_actor->getAabb();
    return self.isContaining(x, y);
}

bool AabbCollider::testCollision(const Aabb& aabb) const
{
    assert(m_actor != nullptr);

    if (!isCollidable())
        return false;

    const Aabb self = m_actor->getAabb();
    return self.isOverlapping(aabb);
}

bool AabbCollider::testCollision(float deltaX, float deltaY, const ICollider* other) const
{
    assert(m_actor != nullptr);

    if (!isCollidableWith(other))
        return false;

    Aabb bounds = m_actor->getAabb();
    bounds.addOffset(deltaX, deltaY);
    return other->testCollision(bounds);
}

// NOTE here we are taking the velocity of the other object
bool AabbCollider::getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const
{
    assert(m_actor != nullptr);

    if (!isCollidable())
        return false;

    const Aabb self = m_actor->getAabb();

    Aabb::Edge edge;
    bool result = self.getCollisionTime(aabb, velX, velY, start, end, edge);
    switch (edge)
    {
    case Aabb::Left:
        normX = -1.f;
        normY = 0.f;
        break;
    case Aabb::Right:
        normX = 1.f;
        normY = 0.f;
        break;
    case Aabb::Bottom:
        normX = 0.f;
        normY = -1.f;
        break;
    case Aabb::Top:
        normX = 0.f;
        normY = 1.f;
        break;
    default:
        break;
    }
    return result;
}

// NOTE here we are taking the velocity of THIS object
// TODO maybe we should just pull these velocities from the actor physics component???
bool AabbCollider::getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const
{
    assert(m_actor != nullptr);
    assert(other != nullptr);

    if (!isCollidableWith(other))
        return false;

    const Aabb bounds = m_actor->getAabb();
    bool result = other->getCollisionTime(bounds, velX, velY, start, end, normX, normY);
    return result;
}
