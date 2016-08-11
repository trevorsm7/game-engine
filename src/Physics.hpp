#pragma once

#include <cstdint>

class Transform;

// TODO combine with Transform?
class Physics
{
    struct {float x, y;} m_vel;
    struct {float x, y;} m_acc;
    float m_mass;
    float m_cor;
    float m_cof;
    uint8_t m_collisions;
    bool m_stuck;

public:
    Physics(float mass=1.f, float cor=1.f, float cof=0.f):
        m_vel{0.f, 0.f}, m_acc{0.f, 0.f}, m_mass(mass), m_cor(cor), m_cof(cof) {}
    ~Physics() {}

    void preUpdate(float delta);
    void update(Transform& transform, float delta);

    void collide(Physics* other, float normX, float normY);

    float getVelX() const {return m_vel.x;}
    float getVelY() const {return m_vel.y;}

    void setVelX(float velX) {m_vel.x = velX;}
    void setVelY(float velY) {m_vel.y = velY;}
    void addVelX(float velX) {m_vel.x += velX;}
    void addVelY(float velY) {m_vel.y += velY;}

    void addAccX(float accX) {m_acc.x += accX;}
    void addAccY(float accY) {m_acc.y += accY;}
};
