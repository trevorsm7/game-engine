#ifndef __ICOLLIDER_H__
#define __ICOLLIDER_H__

class Actor;

class ICollider
{
public:
    //ICollider() {}
    virtual ~ICollider() {}

    // TODO: when would we need to update collider?
    virtual void update(float delta) = 0;

    virtual bool testCollision(float x, float y) = 0;
    //virtual bool testCollision(Actor* actor) = 0;

    virtual void setCollidable(bool collidable) = 0;
};

#endif
