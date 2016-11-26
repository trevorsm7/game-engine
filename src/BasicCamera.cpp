#include "BasicCamera.hpp"
#include "IRenderer.hpp"
#include "Serializer.hpp"
#include "IUserdata.hpp"

#include "lua.hpp"

void BasicCamera::construct(lua_State* L, int index)
{
    const int relIndex = index < 0 ? index - 1 : index;

    IUserdata::getListOpt(L, relIndex, "size", m_width, m_height);
    m_transform.setScale(m_width, m_height);
    m_center.x = m_width * 0.5f;
    m_center.y = m_height * 0.5f;
    IUserdata::getListOpt(L, relIndex, "center", m_center.x, m_center.y);
    IUserdata::getValueOpt(L, relIndex, "fixed", m_fixed);
}

void BasicCamera::serialize(lua_State* L, const char* table, Serializer* serializer, ObjectRef* ref) const
{
    serializer->setList(ref, table, "size", m_width, m_height);
    serializer->setList(ref, table, "center", m_center.x, m_center.y);

    //serializer->setBoolean(ref, table, "fixed", m_fixed);
    if (!m_fixed)
        serializer->setBoolean(ref, table, "fixed", false);
}

void BasicCamera::resize(int width, int height)
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

void BasicCamera::preRender(IRenderer* renderer)
{
    renderer->pushCameraTransform(m_transform);
}

void BasicCamera::postRender(IRenderer* renderer)
{
    renderer->popCameraTransform();
}

void BasicCamera::setCenter(float x, float y)
{
    m_center.x = x;
    m_center.y = y;

    m_transform.setX(x - m_transform.getScaleX() * 0.5f);
    m_transform.setY(y - m_transform.getScaleY() * 0.5f);
}

void BasicCamera::setOrigin(float x, float y)
{
    m_center.x = x + m_transform.getScaleX() * 0.5f;
    m_center.y = y + m_transform.getScaleY() * 0.5f;

    m_transform.setX(x);
    m_transform.setY(y);
}

void BasicCamera::mouseToWorld(const MouseEvent& event, float& x, float& y) const
{
    const float fractionX = float(event.x) / float(event.w);
    const float fractionY = float(event.y) / float(event.h);
    x = fractionX * m_transform.getScaleX() + m_transform.getX();
    y = fractionY * m_transform.getScaleY() + m_transform.getY();
}
