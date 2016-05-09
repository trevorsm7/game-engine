#ifndef __SPRITEGRAPHICS_H__
#define __SPRITEGRAPHICS_H__

#include "IGraphics.h"

#include <string>

class SpriteGraphics : public TGraphics<SpriteGraphics>
{
    std::string m_filename;

    SpriteGraphics() {}

public:
    ~SpriteGraphics() override {}

    void update(float delta) override {}
    void render(IRenderer* renderer) override;

    bool testBounds(float x, float y) const override;

private:
    friend class TGraphics<SpriteGraphics>;
    friend class TUserdata<SpriteGraphics>;
    void construct(lua_State* L);
    void destroy(lua_State* L) {}

    static constexpr const char* const METATABLE = "SpriteGraphics";
    //static constexpr const luaL_Reg METHODS[] = {};
};

#endif
