#include "Actor.hpp"
#include "Canvas.hpp"
#include "IRenderer.hpp"
#include "SpriteGraphics.hpp"
#include "AabbCollider.hpp"
#include "TiledGraphics.hpp"
#include "TiledCollider.hpp"
#include "Physics.hpp"

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

// TODO could be templated at some point (esp. if we store components in a generic array)
// TODO also could make generic addComponent w/ sequential testGraphics, testCollider, etc
void Actor::setGraphics(lua_State* L, int index)
{
    IGraphics* graphics = IGraphics::checkInterface(L, index);

    // Do nothing if we already own the component
    if (m_graphics == graphics)
        return;

    // Clear old component first
    if (m_graphics != nullptr)
    {
        assert(m_graphics->m_actor == this);
        m_graphics->m_actor = nullptr;
        m_graphics->refRemoved(L);
        //m_graphics = nullptr;
    }

    // If component already owned, remove from owner
    // TODO treat as an error or allow components to be shared?
    Actor* const oldActor = graphics->m_actor;
    if (oldActor != nullptr)
    {
        //oldActor->m_graphics->refRemoved(L); // <-
        oldActor->m_graphics = nullptr;
    }
    else // Only increase ref if we don't recycle from an old owner
        graphics->refAdded(L, index); // <-

    // Add component to new actor
    //graphics->refAdded(L, index); // <-
    m_graphics = graphics;
    graphics->m_actor = this;
}

// TODO could be templated at some point (esp. if we store components in a generic array)
// TODO also could make generic addComponent w/ sequential testGraphics, testCollider, etc
void Actor::setCollider(lua_State* L, int index)
{
    ICollider* collider = ICollider::checkInterface(L, index);

    // Do nothing if we already own the component
    if (m_collider == collider)
        return;

    // Clear old component first
    if (m_collider != nullptr)
    {
        assert(m_collider->m_actor == this);
        m_collider->m_actor = nullptr;
        m_collider->refRemoved(L);
        //m_collider = nullptr;
    }

    // If component already owned, remove from owner
    // TODO treat as an error or allow components to be shared?
    Actor* const oldActor = collider->m_actor;
    if (oldActor != nullptr)
    {
        //oldActor->m_collider->refRemoved(L); // <-
        oldActor->m_collider = nullptr;
    }
    else // Only increase ref if we don't recycle from an old owner
        collider->refAdded(L, index); // <-

    // Add component to new actor
    //collider->refAdded(L, index); // <-
    m_collider = collider;
    collider->m_actor = this;
}

// =============================================================================
// Lua library functions
// =============================================================================

void Actor::construct(lua_State* L)
{
    lua_pushliteral(L, "graphics");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setGraphics(L, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "collider");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setCollider(L, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "physics");
    if (lua_rawget(L, 2) != LUA_TNIL)
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
    if (lua_rawget(L, 2) != LUA_TNIL)
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
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        // TODO let graphics/collider define the base size and use this as a scaling factor
        m_transform.setW(luaL_checknumber(L, -2));
        m_transform.setH(luaL_checknumber(L, -1));
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "layer");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setLayer(luaL_checkinteger(L, -1));
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

void Actor::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    if (m_graphics)
    {
        m_graphics->pushUserdata(L);
        serializer->serializeObject(ref, "", "graphics", "setGraphics", L, -1);
        lua_pop(L, 1);
    }

    if (m_collider)
    {
        m_collider->pushUserdata(L);
        serializer->serializeObject(ref, "", "collider", "setCollider", L, -1);
        lua_pop(L, 1);
    }

    // TODO physics; move into Physics?

    // TODO move into Transform?
    // TODO using subtable for testing; maybe keep this way though?
    float position[2] = {m_transform.getX(), m_transform.getY()};
    ref->setArray("transform", "position", position, 2);
    float scale[2] = {m_transform.getW(), m_transform.getH()};
    ref->setArray("transform", "scale", scale, 2);

    ref->setLiteral("", "layer", m_layer);
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

    return 0;
}

int Actor::actor_setGraphics(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);

    actor->setGraphics(L, 2);

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

    return 0;
}

int Actor::actor_setCollider(lua_State* L)
{
    // Validate function arguments
    Actor* actor = Actor::checkUserdata(L, 1);

    actor->setCollider(L, 2);

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
