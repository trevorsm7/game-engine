#ifndef __SPRITECOLLIDER_H__
#define __SPRITECOLLIDER_H__

#include "ICollider.h"
#include "Actor.h"

class SpriteCollider : public ICollider
{
    // TODO: hold reference to IGraphics or Actor?
    // NOTE: at minimum, need Transform...
    Actor* m_actor; // weak_ptr could be handy

public:
    SpriteCollider(Actor* actor): m_actor(actor) {}
    virtual ~SpriteCollider() {}

    void update(float delta) override {}

    bool testCollision(float x, float y) override
    {
        float l = m_actor->m_transform.getX();
        float b = m_actor->m_transform.getY();
        float r = l + m_actor->m_transform.getW();
        float t = b + m_actor->m_transform.getH();
        return x >= l && y >= b && x < r && y < t;
    }
};

#endif
