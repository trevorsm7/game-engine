#pragma once

#include "Transform.hpp"

#include <string>

class TileMap;

class IRenderer
{
public:
    virtual ~IRenderer() {}

    virtual bool init() = 0;
    virtual void preRender() = 0;
    virtual void postRender() = 0;

    virtual void pushModelTransform(Transform& transform) = 0;
    virtual void pushCameraTransform(Transform& transform) = 0;

    virtual void setColor(float red, float green, float blue) = 0;
    virtual void drawSprite(const std::string& name) = 0;
    virtual void drawTiles(TileMap* tilemap) = 0;

    virtual void popModelTransform() = 0;
    virtual void popCameraTransform() = 0;
};
