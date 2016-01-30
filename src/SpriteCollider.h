#ifndef __SPRITECOLLIDER_H__
#define __SPRITECOLLIDER_H__

#include "ICollider.h"
#include "Actor.h"

class SpriteCollider : public ICollider
{
    Actor* m_actor;
    bool m_collidable;

public:
    SpriteCollider(Actor* actor): m_actor(actor), m_collidable(true) {}
    virtual ~SpriteCollider() {}

    void update(float delta) override {}

    bool testCollision(float x, float y) override
    {
        if (!m_collidable)
            return false;

        Transform& transform = m_actor->getTransform();
        float left = transform.getX();
        float bottom = transform.getY();
        float right = left + transform.getW();
        float top = bottom + transform.getH();
        return (x >= left && x < right && y >= bottom && y < top);
    }

    void setCollidable(bool collidable) override {m_collidable = collidable;}
};

#endif
