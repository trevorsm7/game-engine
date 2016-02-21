#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

// TODO: this was done as a quick hack... needs lots of work
// TODO: maybe even just make this a struct?
class Transform
{
    float m_x, m_y;
    float m_w, m_h;

public:
    Transform(): m_x(0.f), m_y(0.f), m_w(1.f), m_h(1.f) {}
    Transform(float x, float y, float w, float h): m_x(x), m_y(y), m_w(w), m_h(h) {}

    float getX() const {return m_x;}
    float getY() const {return m_y;}
    void setX(float x) {m_x = x;}
    void setY(float y) {m_y = y;}

    float getW() const {return m_w;}
    float getH() const {return m_h;}
    void setW(float w) {m_w = w;}
    void setH(float h) {m_h = h;}
};

#endif
