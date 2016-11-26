#pragma once

#include "Transform.hpp"

#include <string>
#include <vector>

class TileMap;

class IRenderer
{
public:
    virtual ~IRenderer() {}

    virtual void preRender() = 0;
    virtual void postRender() = 0;

    virtual void pushModelTransform(Transform& transform) = 0;
    virtual void pushCameraTransform(Transform& transform) = 0;

    virtual void setColor(float red, float green, float blue) = 0;
    virtual void drawSprite(const std::string& name) = 0;
    virtual void drawTiles(const TileMap* tilemap) = 0;
    virtual void drawLines(const std::vector<float>& points) = 0;

    virtual void popModelTransform() = 0;
    virtual void popCameraTransform() = 0;
};
