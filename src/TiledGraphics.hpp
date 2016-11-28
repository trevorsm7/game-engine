#pragma once

#include "IGraphics.hpp"

class TileMap;

class TiledGraphics : public TUserdata<TiledGraphics, IGraphics>
{
    TileMap* m_tilemap;

    TiledGraphics(): m_tilemap(nullptr) {}

public:
    ~TiledGraphics() override {}

    void update(float delta) override {}
    void render(IRenderer* renderer) override;

    bool testBounds(float x, float y, float& xl, float& yl) const override;

    void getSize(float& w, float& h) const override;

private:
    friend class TUserdata<TiledGraphics, IGraphics>;
    void construct(lua_State* L);
    void clone(lua_State* L, TiledGraphics* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_getTileMap(lua_State* L);
    static int script_setTileMap(lua_State* L);

    static constexpr const char* const CLASS_NAME = "TiledGraphics";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"getTileMap", script_getTileMap},
        {"setTileMap", script_setTileMap},
        {nullptr, nullptr}
    };
};
