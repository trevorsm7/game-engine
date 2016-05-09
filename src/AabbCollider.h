#ifndef __AABBCOLLIDER_H__
#define __AABBCOLLIDER_H__

#include "ICollider.h"
#include "Aabb.h"

class AabbCollider : public TCollider<AabbCollider>
{
    AabbCollider() {}

public:
    ~AabbCollider() override {}

    void update(float delta) override {}

    bool testCollision(float x, float y) const override;
    bool testCollision(const Aabb& aabb) const override;
    bool testCollision(float deltaX, float deltaY, const ICollider* other) const override;

    bool getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const override;
    bool getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const override;

private:
    friend class TCollider<AabbCollider>;
    friend class TUserdata<AabbCollider>;
    //void construct(lua_State* L);
    void destroy(lua_State* L) {}

    static constexpr const char* const METATABLE = "AabbCollider";
    //static constexpr const luaL_Reg METHODS[] = {};
};

#endif
