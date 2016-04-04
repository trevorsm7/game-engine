#ifndef __TILEDCOLLIDER_H__
#define __TILEDCOLLIDER_H__

#include "ICollider.h"

#include <string>

class Actor;

class TiledCollider : public ICollider
{
    Actor* m_actor;
    std::string m_tilemap;

public:
    TiledCollider(Actor* actor, std::string tilemap): m_actor(actor), m_tilemap(tilemap) {}
    ~TiledCollider() override {}

    void update(float delta) override {}

    bool testCollision(float x, float y) const override;
    bool testCollision(const Aabb& aabb) const override;
    // NOTE: testing a tilemap against an aabb will be much less effient than vise versa;
    // this would probably only be reasonable when testing a tilemap against a tilemap
    bool testCollision(float deltaX, float deltaY, const ICollider* other) const override;

    bool getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const override;
    bool getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const override;
};

#endif
