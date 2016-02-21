#include "BasicCamera.h"

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

float BasicCamera::cameraToWorldX(int x, int l, int r) const
{
    float fraction = float(x - l) / float(r - l);
    return fraction * m_transform.getW() + m_transform.getX();
}

float BasicCamera::cameraToWorldY(int y, int b, int t) const
{
    float fraction = float(y - b) / float(t - b);
    return fraction * m_transform.getH() + m_transform.getY();
}
