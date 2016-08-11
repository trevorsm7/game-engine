#include "Aabb.h"

#include <limits>
#include <algorithm>

bool Aabb::getCollisionTime(const Aabb& other, float velX, float velY, float& start, float& end, Edge& edge) const
{
    edge = None;

    // TODO can we do this in a simpler way?
    float startX, endX;
    if (velX > 0.f)
    {
        startX = (m_bounds.l - other.m_bounds.r) / velX;
        endX = (m_bounds.r - other.m_bounds.l) / velX;
        edge = Left;
    }
    else if (velX < 0.f)
    {
        startX = (m_bounds.r - other.m_bounds.l) / velX;
        endX = (m_bounds.l - other.m_bounds.r) / velX;
        edge = Right;
    }
    // NOTE using < instead of <= to keep r as exclusive range
    else if (m_bounds.l < other.m_bounds.r && m_bounds.r > other.m_bounds.l)
    {
        startX = std::numeric_limits<float>::lowest();
        endX = std::numeric_limits<float>::max();
    }
    else
        return false;

    float startY, endY;
    if (velY > 0.f)
    {
        startY = (m_bounds.t - other.m_bounds.b) / velY;
        endY = (m_bounds.b - other.m_bounds.t) / velY;
        if (startY >= startX)
            edge = Bottom;
    }
    else if (velY < 0.f)
    {
        startY = (m_bounds.b - other.m_bounds.t) / velY;
        endY = (m_bounds.t - other.m_bounds.b) / velY;
        if (startY >= startX)
            edge = Top;
    }
    else if (m_bounds.t < other.m_bounds.b && m_bounds.b > other.m_bounds.t)
    {
        startY = std::numeric_limits<float>::lowest();
        endY = std::numeric_limits<float>::max();
    }
    else
        return false;

    // Only collide when the greater of the start times is less than the lesser of the end times
    start = std::max(startX, startY);
    end = std::min(endX, endY);
    return start < end;
}
