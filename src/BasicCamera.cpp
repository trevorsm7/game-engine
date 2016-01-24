#include "BasicCamera.h"

void BasicCamera::preRender(IRenderer* renderer)
{
    renderer->pushCameraTransform(m_transform);
}

void BasicCamera::postRender(IRenderer* renderer)
{
    renderer->popCameraTransform();
}

float BasicCamera::cameraToWorldX(int x, int l, int r)
{
    float fraction = float(x - l) / float(r - l);
    return fraction * m_transform.getW() + m_transform.getX();
}

float BasicCamera::cameraToWorldY(int y, int b, int t)
{
    float fraction = float(y - b) / float(t - b);
    return fraction * m_transform.getH() + m_transform.getY();
}
