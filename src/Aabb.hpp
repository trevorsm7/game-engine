#pragma once

class Aabb
{
    struct {float l, t, r, b;} m_bounds;

public:
    Aabb(float l, float t, float r, float b): m_bounds{l, t, r, b} {}

    bool isContaining(float x, float y) const
    {
        return m_bounds.r > x && m_bounds.l <= x && m_bounds.b > y && m_bounds.t <= y;
    }

    bool isOverlapping(const Aabb& other) const
    {
        return m_bounds.r > other.m_bounds.l && m_bounds.l < other.m_bounds.r
            && m_bounds.b > other.m_bounds.t && m_bounds.t < other.m_bounds.b;
    }

    // NOTE here we assume that we are static and the other AABB is moving
    enum Edge {None, Left, Right, Bottom, Top};
    bool getCollisionTime(const Aabb& other, float velX, float velY, float& start, float& end, Edge& edge) const;

    float getLeft() const {return m_bounds.l;}
    float getBottom() const {return m_bounds.b;}
    float getRight() const {return m_bounds.r;}
    float getTop() const {return m_bounds.t;}

    float getWidth() const {return m_bounds.r - m_bounds.l;}
    float getHeight() const {return m_bounds.b - m_bounds.t;}

    void addOffset(float x, float y) {m_bounds.l += x; m_bounds.r += x; m_bounds.b += y; m_bounds.t += y;}
};
