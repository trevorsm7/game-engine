#ifndef __TRANSFORM_H__
#define __TRANSFORM_H__

#include <sstream>

// TODO: this was done as a quick hack... needs lots of work
// TODO: maybe even just make this a struct?
class Transform
{
    float m_x, m_y;
    float m_w, m_h;

public:
    Transform(): m_x(0.f), m_y(0.f), m_w(1.f), m_h(1.f) {}
    Transform(float x, float y, float w, float h): m_x(x), m_y(y), m_w(w), m_h(h) {}

    void moveBy(float x, float y) {m_x += x; m_y += y;}

    float getX() {return m_x;}
    float getY() {return m_y;}
    void setX(float x) {m_x = x;}
    void setY(float y) {m_y = y;}

    float getW() {return m_w;}
    float getH() {return m_h;}
    void setW(float w) {m_w = w;}
    void setH(float h) {m_h = h;}

    std::string getString()
    {
        std::stringstream stream;
        stream << "x: " << m_x << ", y: " << m_y << ", w: " << m_w << ", h: " << m_h;
        return stream.str();
    }
};

#endif
