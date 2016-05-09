#include "BasicCamera.h"
#include "IRenderer.h"

void BasicCamera::resize(int width, int height)
{
    if (m_fixed)
    {
        m_transform.setW(m_width);
        m_transform.setH(m_height);
    }
    else
    {
        float w = float(width);
        float h = float(height);

        if (m_height * w / h > m_width)
        {
            m_transform.setW(m_width);
            m_transform.setH(m_width * h / w);
        }
        else
        {
            m_transform.setW(m_height * w / h);
            m_transform.setH(m_height);
        }
    }

    m_transform.setX(m_center.x - m_transform.getW() * 0.5f);
    m_transform.setY(m_center.y - m_transform.getH() * 0.5f);
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

    m_transform.setX(x - m_transform.getW() * 0.5f);
    m_transform.setY(y - m_transform.getH() * 0.5f);
}

void BasicCamera::mouseToWorld(const MouseEvent& event, float& x, float& y) const
{
    const float fractionX = float(event.x) / float(event.w);
    const float fractionY = float(event.y) / float(event.h);
    x = fractionX * m_transform.getW() + m_transform.getX();
    y = fractionY * m_transform.getH() + m_transform.getY();
}
