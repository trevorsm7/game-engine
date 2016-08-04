#ifndef __SPRITEGRAPHICS_HPP__
#define __SPRITEGRAPHICS_HPP__

#include "IGraphics.hpp"

#include <string>

class SpriteGraphics : public TUserdata<SpriteGraphics, IGraphics>
{
    std::string m_filename;

    SpriteGraphics() {}

public:
    ~SpriteGraphics() override {}

    void update(float delta) override {}
    void render(IRenderer* renderer) override;

    bool testBounds(float x, float y) const override;

private:
    friend class TUserdata<SpriteGraphics, IGraphics>;
    void construct(lua_State* L);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static constexpr const char* const CLASS_NAME = "SpriteGraphics";
    static constexpr const luaL_Reg METHODS[] = {{nullptr, nullptr}};
};

#endif
