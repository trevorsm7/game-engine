#include "Canvas.hpp"
#include "Scene.hpp"
#include "Actor.hpp"
#include "ICamera.hpp"
#include "ICollider.hpp"
#include "Physics.hpp"
#include "Serializer.hpp"

#include <algorithm>
#include <limits>
#include <cmath>
#include <string>

using namespace std::string_literals;

const luaL_Reg Canvas::METHODS[];

ResourceManager* Canvas::getResourceManager() const
{
    if (m_scene)
        return &m_scene->getResourceManager();

    return nullptr;
}

void Canvas::update(lua_State *L, float delta)
{
    if (m_actorRemoved)
    {
        processRemovedActors(L);
        m_actorRemoved = false;
    }

    // Skip the remainder of the update if we are paused
    if (!m_paused)
    {
        // Call user-defined update method
        lua_pushnumber(L, delta);
        pcall(L, "onUpdatePre", 1, 0);

        updatePhysics(L, delta);

        // Order of update dispatch doesn't really matter; choose bottom to top
        for (auto& actor : m_actors)
            actor->update(L, delta);

        // Call user-defined update method
        // TODO is this necessary?
        lua_pushnumber(L, delta);
        pcall(L, "onUpdatePost", 1, 0);
    }

    // Recently added Actors should now be added to the end
    processAddedActors(L);
}

void Canvas::render(IRenderer *renderer)
{
    // Don't render anything if Canvas is not visible
    if (!m_visible)
        return;

    // TODO should camera be allowed to be null?
    if (!m_camera)
        return;

    m_camera->preRender(renderer);

    // TODO: add alternate structure to iterate in sorted order/down quadtree
    // NOTE: rendering most recently added last (on top)
    auto end = m_actors.end();
    for (auto it = m_actors.begin(); it != end; ++it)
    {
        // Skip Actor if it is marked for removal
        if ((*it)->m_canvas != this)
            continue;

        (*it)->render(renderer);
    }

    m_camera->postRender(renderer);
}

bool Canvas::mouseEvent(lua_State *L, MouseEvent& event)
{
    // Don't allow capturing mouse events when Canvas is inactive
    if (!m_visible || m_paused)
        return false;

    // TODO should camera be allowed to be null?
    if (!m_camera)
        return false;

    // Convert click into world coordinates
    // TODO: map a ray/point from camera->world space
    float x, y;
    m_camera->mouseToWorld(event, x, y);

    if (pcallT(L, "onClickPre", false, event.down))
        return true;

    float xl, yl;

    // NOTE: iterate in reverse order of rendering
    auto end = m_actors.rend();
    for (auto it = m_actors.rbegin(); it != end; ++it)
    {
        // Skip Actor if it is marked for removal
        if ((*it)->m_canvas != this)
            continue;

        // Query if click is inside Actor; could be combined with mouseEvent??
        // TODO: Actor should convert ray/point from world->object space
        if (!(*it)->testMouse(x, y, xl, yl))
            continue;

        // TODO: try next actor down, or absorb the click?
        //return (*it)->mouseEvent(L, event.down); // absorb the click
        if ((*it)->mouseEvent(L, event.down, xl, yl))
            return true; // only absorb click if Actor chose to handle it
    }

    if (pcallT(L, "onClickPost", false, event.down))
        return true;

    return false;
}

void Canvas::resize(lua_State* /*L*/, int width, int height)
{
    if (m_camera)
        m_camera->resize(width, height);
}

void Canvas::processAddedActors(lua_State *L)
{
    // Process each actor in the add queue
    for (auto& actor : m_added)
    {
        // Remove actor if it is marked for delete, then skip
        if (actor->m_canvas != this)
        {
            releaseChild(L, actor);
            continue;
        }

        // Find first actor in canvas with greater layer
        // TODO would it be better to search starting from the end instead?
        auto it = m_actors.begin();
        for (auto end = m_actors.end(); it != end; ++it)
            if (actor->getLayer() < (*it)->getLayer())
                break;

        // Insert actor before before one with greater layer (insertion sort)
        m_actors.insert(it, actor);
    }

    // All have been copied from the queue, so we clear it
    m_added.clear();
}

void Canvas::processRemovedActors(lua_State *L)
{
    // Iterate through Actors to find any marked for delete
    auto end = m_actors.end();
    auto tail = m_actors.begin();
    for (auto it = tail; it != end; ++it)
    {
        // Remove actor if it is marked for delete, then skip
        if ((*it)->m_canvas != this)
        {
            releaseChild(L, *it);
            continue;
        }

        // Shift elements back if we have empty space from removals
        if (tail != it)
            *tail = *it;
        ++tail;
    }

    // If any Actors were removed, clear the end of the list
    if (tail != end)
        m_actors.erase(tail, end);
}

void Canvas::updatePhysics(lua_State *L, float delta)
{
    // Update physics state required before collisions
    for (auto& actor : m_actors)
    {
        // Always skip if marked for removal
        if (actor->m_canvas != this)
            continue;

        Physics* physics = actor->getPhysics();
        if (physics)
            physics->preUpdate(delta);
    }

    while (delta > 0.f)
    {
        bool found = false;
        Actor* actor1 = nullptr;
        Actor* actor2 = nullptr;
        float timeStart = delta, timeEnd, normX = 0.f, normY = 0.f;

        // Iterate over all Actors
        auto end = m_actors.end();
        for (auto it = m_actors.begin(); it != end; ++it)
        {
            // Always skip if marked for removal
            if ((*it)->m_canvas != this)
                continue;

            // Get earliest collision among remaining Actors
            // TODO detect and stop repeated collision between stuck objects
            // TODO add stuck flag to objects to treat as static while stuck
            Actor* tempActor2;
            float tempStart, tempEnd, tempNormX, tempNormY;
            if (getEarliestCollision(*it, it+1, end, tempActor2, tempStart, tempEnd, tempNormX, tempNormY))
            {
                // Note if we have found a new earliest collision
                if (tempStart < timeStart)
                {
                    found = true;
                    actor1 = *it;
                    actor2 = tempActor2;
                    timeStart = tempStart;
                    timeEnd = tempEnd;
                    normX = tempNormX;
                    normY = tempNormY;
                }
            }
        }

        // Update all objects to first collision time
        // TODO could avoid updating all objects by simulating reverse movement back to a common time when we check collisions??
        if (timeStart > 0.f)
        {
            for (auto& actor : m_actors)
            {
                Physics* physics = actor->getPhysics();
                if (physics)
                    physics->update(actor->getTransform(), timeStart);
            }

            delta -= timeStart;
        }

        if (found)
        {
            // Allow physics components to resolve collision
            Physics* physics1 = actor1->getPhysics();
            Physics* physics2 = actor2->getPhysics();
            if (physics1)
                physics1->collide(physics2, normX, normY);
            else if (physics2)
                physics2->collide(physics1, normX, normY);

            // Send collision notifications to script
            actor1->collideEvent(L, actor2);
            actor2->collideEvent(L, actor1);
        }
    }
}

bool Canvas::testCollision(float deltaX, float deltaY, const Actor* actor1) const
{
    const ICollider* collider1 = actor1->getCollider();
    if (!collider1 || !collider1->isCollidable())
        return false;

    // TODO: test just added actors as well?
    for (auto& actor2 : m_actors)
    {
        // Always skip if marked for removal
        if (actor2->m_canvas != this)
            continue;

        // Don't allow collision with self
        if (actor1 == actor2)
            continue;

        // Reject early if actor has no collider
        const ICollider* collider2 = actor2->getCollider();
        if (!collider2)
            continue;

        // Test colliders against each other
        if (collider1->testCollision(deltaX, deltaY, collider2))
            return true;
    }

    return false;
}

bool Canvas::getEarliestCollision(const Actor* actor1, ActorIterator it, ActorIterator itEnd, Actor*& hit, float& start, float& end, float& normX, float& normY) const
{
    bool found = false;
    float tempStart, tempEnd;
    float tempNormX, tempNormY;
    start = std::numeric_limits<float>::max();

    // Actor must have a collision component
    const ICollider* collider1 = actor1->getCollider();
    if (!collider1 || !collider1->isCollidable())
        return false;

    // Get Actor's velocity
    const Physics* physics1 = actor1->getPhysics();
    float velX = 0.f, velY = 0.f;
    if (physics1)
    {
        velX = physics1->getVelX();
        velY = physics1->getVelY();
    }

    for (; it != itEnd; ++it)
    {
        Actor* actor2 = *it;

        // Always skip if marked for removal
        if (actor2->m_canvas != this)
            continue;

        // Don't allow collision with self
        if (actor1 == actor2)
            continue;

        // Reject early if actor has no collider
        const ICollider* collider2 = actor2->getCollider();
        if (!collider2)
            continue;

        // Compute the relative velocity (in frame of reference of other object)
        float relVelX = velX, relVelY = velY;
        const Physics* physics2 = actor2->getPhysics();
        if (physics2)
        {
            relVelX -= physics2->getVelX();
            relVelY -= physics2->getVelY();
        }

        // Reject potential collisions between non-moving objects (which we might not be able to resolve)
        if (relVelX == 0.f && relVelY == 0.f)
            continue;

        // Test colliders against each other
        if (collider1->getCollisionTime(relVelX, relVelY, collider2, tempStart, tempEnd, tempNormX, tempNormY))
        {
            // NOTE (tempStart < start || (tempStart == start && relPosY <= 0.f)) orders simultaneous collisions by order in the direction of movement
            // NOTE (tempStart >= 0.f || tempEnd > fabs(tempStart)) is a solution for only considering inward movemnt as collision
            float /*relPosX,*/ relPosY = 0.f;
            if (found)
            {
                // IDEA: return start time(s) in reverse sorted list; compare next in list until no longer equal (would need to store entire list on hit)
                //relPosX = (actor2->getTransform().getX() - hit->getTransform().getX()) * relVelX;
                relPosY = (actor2->getTransform().getY() - hit->getTransform().getY()) * relVelY;
            }
            if (tempEnd > 0.f
                && (tempStart < start || (tempStart == start /*&& relPosX <= 0.f*/ && relPosY <= 0.f))
                && (tempStart >= 0.f || tempEnd > fabs(tempStart)))
            {
                start = tempStart;
                end = tempEnd;
                normX = tempNormX;
                normY = tempNormY;
                hit = actor2;
                found = true;
            }
        }
    }

    return found;
}

// =============================================================================
// Lua library functions
// =============================================================================

void Canvas::construct(lua_State* L)
{
    // TODO make this required?
    lua_pushliteral(L, "camera");
    if (lua_rawget(L, 2) != LUA_TNIL)
        set(L, m_camera, -1);
    lua_pop(L, 1);

    getValueOpt(L, 2, "paused", m_paused);
    getValueOpt(L, 2, "visible", m_visible);
}

void Canvas::clone(lua_State* L, Canvas* source)
{
    // TODO make this required?
    if (source->m_camera)
    {
        source->m_camera->pushClone(L);
        set(L, m_camera, -1);
        lua_pop(L, 1);
    }

    for (auto& actor : source->m_actors)
    {
        if (actor->m_canvas != source)
            continue;

        actor->pushClone(L);
        Actor* ptr = Actor::testUserdata(L, -1);
        assert(ptr != nullptr);
        acquireChild(L, ptr, -1);
        lua_pop(L, 1);
        ptr->m_canvas = this;
        m_actors.push_back(ptr);
    }

    for (auto& actor : source->m_added)
    {
        if (actor->m_canvas != source)
            continue;

        actor->pushClone(L);
        Actor* ptr = Actor::testUserdata(L, -1);
        assert(ptr != nullptr);
        acquireChild(L, ptr, -1);
        lua_pop(L, 1);
        ptr->m_canvas = this;
        m_added.push_back(ptr);
    }

    m_paused = source->m_paused;
    m_visible = source->m_visible;
}

void Canvas::destroy(lua_State* /*L*/)
{
    // Mark each Actor in primary list for removal
    for (auto& actor : m_actors)
    {
        if (actor->m_canvas == this)
            actor->m_canvas = nullptr;
        //releaseChild(L, actor); // children automatically released on destroy
    }

    // Mark each actor in pending list for removal
    for (auto& actor : m_added)
    {
        if (actor->m_canvas == this)
            actor->m_canvas = nullptr;
        //releaseChild(L, actor); // children automatically released on destroy
    }
}

void Canvas::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    for (auto& actor : m_actors)
    {
        if (actor->m_canvas != this)
            continue;

        serializer->serializeMember(ref, "", "", "addActor", L, actor);
    }

    for (auto& actor : m_added)
    {
        if (actor->m_canvas != this)
            continue;

        serializer->serializeMember(ref, "", "", "addActor", L, actor);
    }

    serializer->serializeMember(ref, "", "camera", "setCamera", L, m_camera);

    serializer->setBoolean(ref, "", "paused", m_paused);
    serializer->setBoolean(ref, "", "visible", m_visible);
}

int Canvas::canvas_addActor(lua_State *L)
{
    // Validate function arguments
    Canvas* canvas = Canvas::checkUserdata(L, 1);
    Actor* actor = Actor::checkUserdata(L, 2);

    if (actor->m_canvas == canvas)
        return 0;

    // Don't allow adding an Actor that already belongs to another Canvas
    // NOTE: while we could implicitly remove, might be better to throw an error
    //luaL_argcheck(L, (actor->m_canvas == nullptr), 2, "already belongs to a Canvas\n");

    // Check if Actor already added (a previous removal was still pending)
    // NOTE: this will prevent duplicates and faulty ref counts
    auto& actors = canvas->m_actors;
    auto& added = canvas->m_added;
    if (std::find(actors.begin(), actors.end(), actor) == actors.end() &&
        std::find(added.begin(), added.end(), actor) == added.end())
    {
        // Queue up the add; will take affect after the update loop
        canvas->m_added.push_back(actor);
        canvas->acquireChild(L, actor, 2);
    }

    // Finally, mark Actor as added to this Canvas
    actor->m_canvas = canvas;

    return 0;
}

int Canvas::canvas_removeActor(lua_State *L)
{
    // Validate function arguments
    Canvas* canvas = Canvas::checkUserdata(L, 1);
    Actor* actor = Actor::checkUserdata(L, 2);

    // Actor must belong to this Canvas before we can remove it obviously...
    luaL_argcheck(L, (actor->m_canvas == canvas), 2, "doesn't belong to this Canvas\n");
    //bool isOwner = (actor->m_canvas == canvas);
    bool isOwner = true;

    // Mark Actor for later removal (after we're done iterating)
    if (isOwner)
    {
        canvas->m_actorRemoved = true;
        actor->m_canvas = nullptr;
    }

    lua_pushboolean(L, isOwner);
    return 1;
}

int Canvas::canvas_clear(lua_State* L)
{
    // Validate function arguments
    Canvas *canvas = Canvas::checkUserdata(L, 1);

    // Mark each Actor belonging to primary list for removal
    for (auto& actor : canvas->m_actors)
        if (actor->m_canvas == canvas)
            actor->m_canvas = nullptr;

    // Mark each actor belonging to pending list for removal
    for (auto& actor : canvas->m_added)
        if (actor->m_canvas == canvas)
            actor->m_canvas = nullptr;

    // Notify Canvas that Actors are marked for removal
    canvas->m_actorRemoved = true;

    return 0;
}

int Canvas::canvas_getCamera(lua_State* L)
{
    Canvas* canvas = Canvas::checkUserdata(L, 1);
    return pushMember(L, canvas->m_camera);
}

int Canvas::canvas_setCamera(lua_State* L)
{
    Canvas* canvas = Canvas::checkUserdata(L, 1);
    canvas->set(L, canvas->m_camera, 2);
    return 0;
}

int Canvas::canvas_setCenter(lua_State* L)
{
    // Validate function arguments
    Canvas *canvas = Canvas::checkUserdata(L, 1);

    // Get position either through Actor or directly from argumentsS
    float x, y;
    if (lua_isuserdata(L, 2))
    {
        Actor *actor = Actor::checkUserdata(L, 2);
        actor->getTransform().getCenter(x, y);
    }
    else
    {
        x = static_cast<float>(luaL_checknumber(L, 2));
        y = static_cast<float>(luaL_checknumber(L, 3));
    }

    if (canvas->m_camera)
        canvas->m_camera->setCenter(x, y);

    return 0;
}

int Canvas::canvas_setOrigin(lua_State* L)
{
    // Validate function arguments
    Canvas *canvas = Canvas::checkUserdata(L, 1);
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    if (canvas->m_camera)
        canvas->m_camera->setOrigin(x, y);

    return 0;
}

int Canvas::canvas_getCollision(lua_State* L)
{
    // Validate function arguments
    Canvas *canvas = Canvas::checkUserdata(L, 1);
    float x = static_cast<float>(luaL_checknumber(L, 2));
    float y = static_cast<float>(luaL_checknumber(L, 3));

    // TODO return list of collisions instead of just first collision?
    for (auto& actor : canvas->m_actors)
    {
        // Always skip if marked for removal
        if (actor->m_canvas != canvas)
            continue;

        // If we find a collision, push the Actor and return
        if (actor->testCollision(x, y))
        {
            actor->pushUserdata(L);
            return 1;
        }
    }

    // NOTE: newly added Actors SHOULD be eligible for collision
    for (auto& actor : canvas->m_added)
    {
        // Always skip if marked for removal
        if (actor->m_canvas != canvas)
            continue;

        // If we find a collision, push the Actor and return
        if (actor->testCollision(x, y))
        {
            actor->pushUserdata(L);
            return 1;
        }
    }

    return 0;
}

int Canvas::canvas_setPaused(lua_State *L)
{
    // Validate function arguments
    Canvas *canvas = Canvas::checkUserdata(L, 1);
    luaL_argcheck(L, lua_isboolean(L, 2), 2, "must be boolean (true to pause)\n");

    canvas->m_paused = (lua_toboolean(L, 2) == 1);

    return 0;
}

int Canvas::canvas_setVisible(lua_State *L)
{
    // Validate function arguments
    Canvas *canvas = Canvas::checkUserdata(L, 1);
    luaL_argcheck(L, lua_isboolean(L, 2), 2, "must be boolean (true for visible)\n");

    canvas->m_visible = (lua_toboolean(L, 2) == 1);

    return 0;
}
