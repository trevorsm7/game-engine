#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include "Aabb.h"

// TODO: this was done as a quick hack... needs lots of work
// TODO: maybe even just make this a struct?
class Transform
{
    float m_x, m_y;
    float m_w, m_h;

public:
    Transform(): m_x(0.f), m_y(0.f), m_w(1.f), m_h(1.f) {}
    Transform(float x, float y, float w, float h): m_x(x), m_y(y), m_w(w), m_h(h) {}

    Aabb getAabb() const {return Aabb(m_x, m_y, m_x + m_w, m_y + m_h);}

    float getX() const {return m_x;}
    float getY() const {return m_y;}
    float getW() const {return m_w;}
    float getH() const {return m_h;}

    void getCenter(float& x, float& y) const {x = m_x + m_w * 0.5f; y = m_y + m_h * 0.5f;}

    void setX(float x) {m_x = x;}
    void setY(float y) {m_y = y;}
    void setW(float w) {m_w = w;}
    void setH(float h) {m_h = h;}

    void addX(float x) {m_x += x;}
    void addY(float y) {m_y += y;}
};

#endif
