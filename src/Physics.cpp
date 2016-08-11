#include "Physics.hpp"
#include "Transform.hpp"

#include <cmath>
#include <algorithm>

void Physics::preUpdate(float delta)
{
    // Apply acceleration to velocity at start of frame
    m_vel.x += m_acc.x * delta;
    m_vel.y += m_acc.y * delta;
    m_acc.x = 0.f;
    m_acc.y = 0.f;
    m_collisions = 0;
    m_stuck = false;
}

void Physics::update(Transform& transform, float delta)
{
    transform.addX(m_vel.x * delta);
    transform.addY(m_vel.y * delta);
    //m_collisions = 0;
}

void Physics::collide(Physics* other, float normX, float normY)
{
    // TODO might be more effective to apply motion constraints when we hit static objects
    if (m_collisions >= 4 && (!other || other->m_collisions >= 4))// || other->m_stuck))
    {
        m_stuck = true;
        //m_vel.x = 0.f;
        //m_vel.y = 0.f;
        // NOTE this might only get applied to the first object
        const float dot = m_vel.x * normX + m_vel.y * normY;
        m_vel.x -= normX * dot;
        m_vel.y -= normY * dot;
    }
    ++m_collisions;

    if (m_stuck)
    {
        if (other && !other->m_stuck)
            other->collide(this, normX, normY);
        return;
    }

    float relVelX = m_vel.x;
    float relVelY = m_vel.y;

    float cor = m_cor;
    float cof = m_cof;
    if (other)
    {
        // Get velocity of obj1 in the frame of ref of obj2
        relVelX -= other->m_vel.x;
        relVelY -= other->m_vel.y;

        cor *= other->m_cor;
        cof *= other->m_cof;

        ++other->m_collisions;
    }
    // HACK remove COR when colliding infinite mass with static
    else if (isinf(m_mass))
    {
        cor = 0.f;
    }

    // Get the component of velocity in the normal direction
    const float dot = relVelX * normX + relVelY * normY;
    const float normVelX = normX * dot;
    const float normVelY = normY * dot;

    // Get remaining (tangential) component of velocity
    //const float tanVelX = relVelX - normVelX;
    //const float tanVelY = relVelY - normVelY;

    // Apply restitution coefficient to get rebound velocity
    const float restitutionX = normVelX * (1 + cor);
    const float restitutionY = normVelY * (1 + cor);

    /*float frictionX = std::min(fabsf(tanVelX), fabsf(cof * restitutionY));
    float frictionY = std::min(fabsf(tanVelY), fabsf(cof * restitutionX));
    if (tanVelX * frictionX < 0.f)
        frictionX = -frictionX;
    if (tanVelY * frictionY < 0.f)
        frictionY = -frictionY;*/

    if (other && !other->m_stuck)
    {
        // TODO handle infinite mass explicitly
        const float m1 = m_mass;
        const float m2 = other->m_mass;
        float ratio1, ratio2;
        if (isinf(m1))
        {
            if (isinf(m2))
            {
                ratio1 = 0.5f;
                ratio2 = 0.5f;
            }
            else
            {
                ratio1 = 0.f;
                ratio2 = 1.f;
            }
        }
        else if (isinf(m2))
        {
            ratio1 = 1.f;
            ratio2 = 0.f;
        }
        else
        {
            ratio1 = m2 / (m1 + m2);
            ratio2 = m1 / (m1 + m2);
        }

        // Subtract a fraction of the collision velocity from obj1
        //m_vel.x -= (restitutionX + frictionX) * ratio1;
        //m_vel.y -= (restitutionY + frictionY) * ratio1;
        m_vel.x -= restitutionX * ratio1;
        m_vel.y -= restitutionY * ratio1;

        // Add a fraction of the collision velocity to obj2
        //other->addVelX((restitutionX + frictionX) * ratio2);
        //other->addVelY((restitutionY + frictionY) * ratio2);
        other->addVelX(restitutionX * ratio2);
        other->addVelY(restitutionY * ratio2);
    }
    else
    {
        // Subtract the collision velocity from obj1
        // Use no friction with static objects for now...
        m_vel.x -= restitutionX;// + frictionX;
        m_vel.y -= restitutionY;// + frictionY;
    }
}
