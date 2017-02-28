#pragma once

#include "ICollider.hpp"
#include "Aabb.hpp"

class AabbCollider : public TUserdata<AabbCollider, ICollider>
{
    AabbCollider() {}

public:
    ~AabbCollider() override {}

    void update(float /*delta*/) override {}

    bool testCollision(float x, float y) const override;
    bool testCollision(const Aabb& aabb) const override;
    bool testCollision(float deltaX, float deltaY, const ICollider* other) const override;

    bool getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const override;
    bool getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const override;

private:
    friend class TUserdata<AabbCollider, ICollider>;
    //void construct(lua_State* L) {}
    //void destroy(lua_State* L) {}
    //void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref) {}

    static constexpr const char* const CLASS_NAME = "AabbCollider";
    static constexpr const luaL_Reg METHODS[] = {{nullptr, nullptr}};
};
