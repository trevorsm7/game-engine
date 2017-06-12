#include "Camera2D.hpp"
#include "IRenderer.hpp"
#include "Serializer.hpp"

#include "lua.hpp"

const luaL_Reg Camera2D::METHODS[];

void Camera2D::construct(lua_State* L)
{
    getListOpt(L, 2, "size", m_width, m_height);
    m_transform.setScale(m_width, m_height);
    m_center.x = m_width * 0.5f;
    m_center.y = m_height * 0.5f;
    getListOpt(L, 2, "center", m_center.x, m_center.y);
    getValueOpt(L, 2, "fixed", m_fixed);
}

void Camera2D::clone(lua_State* L, Camera2D* source)
{
    m_width = source->m_width;
    m_height = source->m_height;
    m_center = source->m_center;
    m_fixed = source->m_fixed;
}

void Camera2D::serialize(lua_State* /*L*/, Serializer* serializer, ObjectRef* ref)
{
    serializer->setList(ref, "", "size", m_width, m_height);
    serializer->setList(ref, "", "center", m_center.x, m_center.y);
    serializer->setBoolean(ref, "", "fixed", m_fixed);
}

void Camera2D::resize(int width, int height)
{
    if (m_fixed)
    {
        m_transform.setScale(m_width, m_height);
    }
    else
    {
        float w = float(width);
        float h = float(height);

        if (m_height * w / h > m_width)
        {
            m_transform.setScale(m_width, m_width * h / w);
        }
        else
        {
            m_transform.setScale(m_height * w / h, m_height);
        }
    }

    m_transform.setX(m_center.x - m_transform.getScaleX() * 0.5f);
    m_transform.setY(m_center.y - m_transform.getScaleY() * 0.5f);
}

void Camera2D::preRender(IRenderer* renderer)
{
    renderer->pushCameraTransform(m_transform);
}

void Camera2D::postRender(IRenderer* renderer)
{
    renderer->popCameraTransform();
}

void Camera2D::setCenter(float x, float y)
{
    m_center.x = x;
    m_center.y = y;

    m_transform.setX(x - m_transform.getScaleX() * 0.5f);
    m_transform.setY(y - m_transform.getScaleY() * 0.5f);
}

void Camera2D::setOrigin(float x, float y)
{
    m_center.x = x + m_transform.getScaleX() * 0.5f;
    m_center.y = y + m_transform.getScaleY() * 0.5f;

    m_transform.setX(x);
    m_transform.setY(y);
}

void Camera2D::mouseToWorld(const MouseEvent& event, float& x, float& y) const
{
    const float fractionX = float(event.x) / float(event.w);
    const float fractionY = float(event.y) / float(event.h);
    x = fractionX * m_transform.getScaleX() + m_transform.getX();
    y = fractionY * m_transform.getScaleY() + m_transform.getY();
}
