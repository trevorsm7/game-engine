#include "SpriteGraphics.hpp"
#include "Serializer.hpp"
#include "IRenderer.hpp"
#include "Actor.hpp"

#include <cassert>

const luaL_Reg SpriteGraphics::METHODS[];

void SpriteGraphics::render(IRenderer* renderer)
{
    if (!isVisible())
        return;

    renderer->setColor(m_color.r, m_color.g, m_color.b);
    renderer->drawSprite(m_filename);
}

void SpriteGraphics::construct(lua_State* L)
{
    getStringReq(L, 2, "sprite", m_filename);
}

void SpriteGraphics::clone(lua_State* /*L*/, SpriteGraphics* source)
{
    m_filename = source->m_filename;
}

void SpriteGraphics::serialize(lua_State* /*L*/, Serializer* serializer, ObjectRef* ref)
{
    serializer->setString(ref, "", "sprite", m_filename);
}
