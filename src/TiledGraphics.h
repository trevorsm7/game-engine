#ifndef __TILEDGRAPHICS_H__
#define __TILEDGRAPHICS_H__

#include "IGraphics.h"

#include <string>

class IRenderer;
class TileMap;

class TiledGraphics : public TGraphics<TiledGraphics>
{
    TileMap* m_tilemap;

    TiledGraphics(): m_tilemap(nullptr) {}

public:
    ~TiledGraphics() override {}

    void update(float delta) override {}
    void render(IRenderer* renderer) override;

    bool testBounds(float x, float y) const override;

private:
    friend class TGraphics<TiledGraphics>;
    friend class TUserdata<TiledGraphics>;
    void construct(lua_State* L);
    void destroy(lua_State* L);

    static constexpr const char* const METATABLE = "TiledGraphics";
    static int script_getTileMap(lua_State* L);
    static constexpr const luaL_Reg METHODS[] =
    {
        {"getTileMap", script_getTileMap},
        {nullptr, nullptr}
    };
};

#endif
