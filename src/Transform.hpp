#pragma once

#include "Aabb.hpp"

struct lua_State;
class Serializer;
class ObjectRef;

// TODO: this was done as a quick hack... needs lots of work
// TODO: maybe even just make this a struct?
class Transform
{
    float m_x, m_y;
    float m_sx, m_sy;

public:
    Transform(): m_x(0.f), m_y(0.f), m_sx(1.f), m_sy(1.f) {}
    Transform(float x, float y, float sx, float sy): m_x(x), m_y(y), m_sx(sx), m_sy(sy) {}

    void construct(lua_State* L, int index);
    void serialize(lua_State* L, const char* table, Serializer* serializer, ObjectRef* ref) const;

    float getX() const {return m_x;}
    float getY() const {return m_y;}
    float getScaleX() const {return m_sx;}
    float getScaleY() const {return m_sy;}

    void getCenter(float& x, float& y) const {x = m_x + m_sx * 0.5f; y = m_y + m_sy * 0.5f;}

    void setX(float x) {m_x = x;}
    void setY(float y) {m_y = y;}
    void setPosition(float x, float y) {m_x = x; m_y = y;}
    void setScale(float sx, float sy) {m_sx = sx; m_sy = sy;}

    void addX(float x) {m_x += x;}
    void addY(float y) {m_y += y;}
};
