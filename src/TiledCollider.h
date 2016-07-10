#ifndef __TILEDCOLLIDER_H__
#define __TILEDCOLLIDER_H__

#include "ICollider.h"

#include <string>

class Actor;
class TileMap;

class TiledCollider : public TCollider<TiledCollider>
{
    TileMap* m_tilemap;

    TiledCollider(): m_tilemap(nullptr) {}

public:
    ~TiledCollider() override {}

    void update(float delta) override {}

    bool testCollision(float x, float y) const override;
    bool testCollision(const Aabb& aabb) const override;
    // NOTE: testing a tilemap against an aabb will be much less effient than vise versa;
    // this would probably only be reasonable when testing a tilemap against a tilemap
    bool testCollision(float deltaX, float deltaY, const ICollider* other) const override;

    bool getCollisionTime(const Aabb& aabb, float velX, float velY, float& start, float& end, float& normX, float& normY) const override;
    bool getCollisionTime(float velX, float velY, const ICollider* other, float& start, float& end, float& normX, float& normY) const override;

private:
    void setTileMap(lua_State* L, int index);

private:
    friend class TCollider<TiledCollider>;
    friend class TUserdata<TiledCollider>;
    void construct(lua_State* L);
    void destroy(lua_State* L);

    static constexpr const char* const METATABLE = "TiledCollider";
    static int script_getTileMap(lua_State* L);
    static int script_setTileMap(lua_State* L);
    static constexpr const luaL_Reg METHODS[] =
    {
        {"getTileMap", script_getTileMap},
        {"setTileMap", script_setTileMap},
        {nullptr, nullptr}
    };
};

#endif
