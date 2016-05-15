#include "Actor.h"
#include "Canvas.h"
#include "IRenderer.h"
#include "SpriteGraphics.h"
#include "AabbCollider.h"
#include "TiledGraphics.h"
#include "TiledCollider.h"
#include "Physics.h"

#include <limits>
#include <cstdint>
#include <cassert>

ResourceManager* Actor::getResourceManager() const
{
    if (m_canvas)
        return m_canvas->getResourceManager();

    return nullptr;
}

void Actor::update(lua_State* L, float delta)
{
    lua_pushnumber(L, delta);
    pcall(L, "update", 1, 0);

    if (m_graphics)
        m_graphics->update(delta);

    // NOTE this should be, for example, related to resizing bounds to reflect animation; not related to collisions
    if (m_collider)
        m_collider->update(delta);
}

void Actor::render(IRenderer* renderer)
{
    // TODO check first if not visible and return early if so?
    assert(renderer != nullptr);
    renderer->pushModelTransform(m_transform);
    if (m_graphics)
        m_graphics->render(renderer);
    // TODO: render children? need to check if visible?
    renderer->popModelTransform();
}

bool Actor::mouseEvent(lua_State* L, bool down)//MouseEvent& event)
{
    bool handled = false;

    lua_pushboolean(L, down);
    if (pcall(L, "mouse", 1, 1))
    {
        handled = lua_isboolean(L, -1) && lua_toboolean(L, -1);
        lua_pop(L, 1);
    }

    return handled;
}

void Actor::collideEvent(lua_State* L, Actor* with)
{
    // Push other Actor's full userdata on stack
    assert(with != nullptr);
    with->pushUserdata(L);
    pcall(L, "collided", 1, 0);
}

bool Actor::testMouse(float x, float y) const
{
    if (!m_graphics)
        return false;

    return m_graphics->testBounds(x, y);
}

bool Actor::testCollision(float x, float y) const
{
    // NOTE: should invisible actors still be collidable? maybe
    if (!m_collider)
        return false;

    return m_collider->testCollision(x, y);
}

// =============================================================================
// Lua library functions
// =============================================================================

// NOTE constexpr declaration requires a definition
const luaL_Reg Actor::METHODS[];

void Actor::construct(lua_State* L)
{
    // Add empty table as uservalue for storage of runtime variables
    lua_newtable(L);
    lua_setuservalue(L, -2);

    // Create the Actor object
    // TODO: what about looping of given params instead of checking all?
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_pushliteral(L, "graphics");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        m_graphics = IGraphics::checkUserdata(L, -1);
        if (m_graphics->m_actor != nullptr)
            m_graphics->m_actor->m_graphics = nullptr;
        else
            m_graphics->refAdded(L, -1);
        m_graphics->m_actor = this;
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "collider");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        m_collider = ICollider::checkUserdata(L, -1);
        if (m_collider->m_actor != nullptr)
            m_collider->m_actor->m_collider = nullptr;
        else
            m_collider->refAdded(L, -1);
        m_collider->m_actor = this;
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "physics");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);

        float mass = 1.f, cor = 1.f, cof = 0.f;

        lua_pushliteral(L, "mass");
        if (lua_rawget(L, -2) != LUA_TNIL)
        {
            luaL_checktype(L, -1, LUA_TNUMBER);
            mass = lua_tonumber(L, -1);
        }
        lua_pop(L, 1);

        lua_pushliteral(L, "cor");
        if (lua_rawget(L, -2) != LUA_TNIL)
        {
            luaL_checktype(L, -1, LUA_TNUMBER);
            cor = lua_tonumber(L, -1);
        }
        lua_pop(L, 1);

        lua_pushliteral(L, "cof");
        if (lua_rawget(L, -2) != LUA_TNIL)
        {
            luaL_checktype(L, -1, LUA_TNUMBER);
            cof = lua_tonumber(L, -1);
        }
        lua_pop(L, 1);

        m_physics = PhysicsPtr(new Physics(mass, cor, cof));
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "position");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        m_transform.setX(luaL_checknumber(L, -2));
        m_transform.setY(luaL_checknumber(L, -1));
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "scale");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        m_transform.setW(luaL_checknumber(L, -2));
        m_transform.setH(luaL_checknumber(L, -1));
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "layer");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TNUMBER);
        setLayer(lua_tointeger(L, -1));
    }
    lua_pop(L, 1);
}

void Actor::destroy(lua_State* L)
{
    if (m_graphics)
    {
        m_graphics->m_actor = nullptr;
        m_graphics->refRemoved(L);
        m_graphics = nullptr;
    }

    if (m_collider)
    {
        m_collider->m_actor = nullptr;
        m_collider->refRemoved(L);
        m_collider = nullptr;
    }
}

int Actor::actor_getCanvas(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);

    if (actor->m_canvas != nullptr)
    {
        actor->m_canvas->pushUserdata(L);
        return 1;
    }

    // NOTE will implicitly return nil, correct?
    return 0;
}

int Actor::actor_getGraphics(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);

    if (actor->m_graphics != nullptr)
    {
        actor->m_graphics->pushUserdata(L);
        return 1;
    }

    // NOTE will implicitly return nil, correct?
    return 0;
}

int Actor::actor_getCollider(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);

    if (actor->m_collider != nullptr)
    {
        actor->m_collider->pushUserdata(L);
        return 1;
    }

    // NOTE will implicitly return nil, correct?
    return 0;
}

int Actor::actor_getPosition(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);

    lua_pushnumber(L, actor->m_transform.getX());
    lua_pushnumber(L, actor->m_transform.getY());
    // TODO: return Z?

    return 2;
}

int Actor::actor_setPosition(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    actor->m_transform.setX(x);
    actor->m_transform.setY(y);

    // Return self userdata
    lua_pushvalue(L, 1);
    return 1;
}

int Actor::actor_setScale(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);
    float w = static_cast<float>(luaL_checknumber(L, 2));
    float h = static_cast<float>(luaL_checknumber(L, 3));

    actor->m_transform.setW(w);
    actor->m_transform.setH(h);

    // Return self userdata
    lua_pushvalue(L, 1);
    return 1;
}

int Actor::actor_testCollision(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);
    float deltaX = static_cast<float>(luaL_checknumber(L, 2));
    float deltaY = static_cast<float>(luaL_checknumber(L, 3));

    bool result = false;
    if (actor->m_collider && actor->m_canvas)
        result = actor->m_canvas->testCollision(deltaX, deltaY, actor);

    lua_pushboolean(L, result);

    return 1;
}

/*int Actor::actor_getEarliestCollision(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);

    Actor* hit = nullptr;
    float start = std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::max();;

    bool result = false;
    if (actor->m_collider && actor->m_canvas)
        result = actor->m_canvas->getEarliestCollision(actor, hit, start, end);

    lua_pushboolean(L, result);
    if (hit != nullptr)
    {
        hit->pushUserdata();
        lua_pushnumber(L, start);
        lua_pushnumber(L, end);
        return 4;
    }

    return 1;
}*/

int Actor::actor_setVelocity(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    if (actor->m_physics)
    {
        actor->m_physics->setVelX(x);
        actor->m_physics->setVelY(y);
    }

    // Return self userdata
    lua_pushvalue(L, 1);
    return 1;
}

int Actor::actor_getVelocity(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);

    Physics* physics = actor->m_physics.get();
    if (physics)
    {
        lua_pushnumber(L, physics->getVelX());
        lua_pushnumber(L, physics->getVelY());
    }
    else
    {
        lua_pushnumber(L, 0.);
        lua_pushnumber(L, 0.);
    }

    return 2;
}

int Actor::actor_addAcceleration(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    if (actor->m_physics)
    {
        actor->m_physics->addAccX(x);
        actor->m_physics->addAccY(y);
    }

    // Return self userdata
    lua_pushvalue(L, 1);
    return 1;
}
