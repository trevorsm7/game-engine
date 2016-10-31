#include "Actor.hpp"
#include "Canvas.hpp"
#include "IRenderer.hpp"
#include "IGraphics.hpp"
#include "ICollider.hpp"
#include "IPathing.hpp"
#include "Physics.hpp"
#include "Serializer.hpp"

#include <limits>
#include <cstdint>
#include <cassert>

const luaL_Reg Actor::METHODS[];

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

    // TODO this is for debugging only; remove
    if (m_pathing)
        m_pathing->render(renderer);

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

void Actor::construct(lua_State* L)
{
    lua_pushliteral(L, "graphics");
    if (lua_rawget(L, 2) != LUA_TNIL)
        set(L, m_graphics, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "collider");
    if (lua_rawget(L, 2) != LUA_TNIL)
        set(L, m_collider, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "pathing");
    if (lua_rawget(L, 2) != LUA_TNIL)
        set(L, m_pathing, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "physics");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        m_physics = PhysicsPtr(new Physics());
        m_physics->construct(L, -1);
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "transform");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        m_transform.construct(L, -1);
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "layer");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setLayer(luaL_checkinteger(L, -1));
    lua_pop(L, 1);
}

void Actor::clone(lua_State* L, Actor* source)
{
    if (source->m_graphics)
    {
        source->m_graphics->pushClone(L);
        set(L, m_graphics, -1);
        lua_pop(L, 1);
    }

    if (source->m_collider)
    {
        source->m_collider->pushClone(L);
        set(L, m_collider, -1);
        lua_pop(L, 1);
    }

    if (source->m_pathing)
    {
        source->m_pathing->pushClone(L);
        set(L, m_pathing, -1);
        lua_pop(L, 1);
    }

    if (source->m_physics)
        m_physics = PhysicsPtr(new Physics(*source->m_physics));

    m_transform = source->m_transform;
    m_layer = source->m_layer;
}

void Actor::destroy(lua_State* L)
{
    remove(L, m_graphics);
    remove(L, m_collider);
    remove(L, m_pathing);
}

void Actor::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    if (m_graphics)
    {
        m_graphics->pushUserdata(L);
        serializer->serializeMember(ref, "", "graphics", "setGraphics", L, -1);
        lua_pop(L, 1);
    }

    if (m_collider)
    {
        m_collider->pushUserdata(L);
        serializer->serializeMember(ref, "", "collider", "setCollider", L, -1);
        lua_pop(L, 1);
    }

    if (m_pathing)
    {
        m_pathing->pushUserdata(L);
        serializer->serializeMember(ref, "", "pathing", "setPathing", L, -1);
        lua_pop(L, 1);
    }

    if (m_physics)
        m_physics->serialize(L, "physics", serializer, ref);

    m_transform.serialize(L, "transform", serializer, ref);

    serializer->setNumber(ref, "", "layer", m_layer);
}

int Actor::actor_getCanvas(lua_State* L)
{
    Actor* actor = Actor::checkUserdata(L, 1);
    return pushMember(L, actor->m_canvas);
}

int Actor::actor_getGraphics(lua_State* L)
{
    Actor* actor = Actor::checkUserdata(L, 1);
    return pushMember(L, actor->m_graphics);
}

int Actor::actor_setGraphics(lua_State* L)
{
    Actor* actor = Actor::checkUserdata(L, 1);
    actor->set(L, actor->m_graphics, 2);
    return 0;
}

int Actor::actor_getCollider(lua_State* L)
{
    Actor* actor = Actor::checkUserdata(L, 1);
    return pushMember(L, actor->m_collider);
}

int Actor::actor_setCollider(lua_State* L)
{
    Actor* actor = Actor::checkUserdata(L, 1);
    actor->set(L, actor->m_collider, 2);
    return 0;
}

int Actor::actor_getPathing(lua_State* L)
{
    Actor* actor = Actor::checkUserdata(L, 1);
    return pushMember(L, actor->m_pathing);
}

int Actor::actor_setPathing(lua_State* L)
{
    Actor* actor = Actor::checkUserdata(L, 1);
    actor->set(L, actor->m_pathing, 2);
    return 0;
}

int Actor::actor_getPosition(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);

    lua_pushnumber(L, actor->m_transform.getX());
    lua_pushnumber(L, actor->m_transform.getY());
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

    return 0;
}

int Actor::actor_setScale(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);
    float w = static_cast<float>(luaL_checknumber(L, 2));
    float h = static_cast<float>(luaL_checknumber(L, 3));

    actor->m_transform.setW(w);
    actor->m_transform.setH(h);

    return 0;
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

    return 0;
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

    return 0;
}
