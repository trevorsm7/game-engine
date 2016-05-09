#ifndef __TILEDGRAPHICS_H__
#define __TILEDGRAPHICS_H__

#include "IGraphics.h"

#include <string>

class IRenderer;

class TiledGraphics : public TGraphics<TiledGraphics>
{
    std::string m_tilemap;

    TiledGraphics() {}

public:
    ~TiledGraphics() override {}

    void update(float delta) override {}
    void render(IRenderer* renderer) override;

    bool testBounds(float x, float y) const override;

private:
    friend class TGraphics<TiledGraphics>;
    friend class TUserdata<TiledGraphics>;
    void construct(lua_State* L);
    void destroy(lua_State* L) {}

    static constexpr const char* const METATABLE = "TiledGraphics";
    //static constexpr const luaL_Reg METHODS[] = {};
};

#endif
