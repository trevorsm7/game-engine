#ifndef __DEBUGRENDERER_H__
#define __DEBUGRENDERER_H__

#include "IRenderer.h"

#include <cstdio>

class DebugRenderer : public IRenderer
{
public:
    ~DebugRenderer() override {}

    void init() override {printf("DebugRenderer::init()\n");}
    void preRender() override {printf("DebugRenderer::preRender()\n");}
    void postRender() override {printf("DebugRenderer::postRender()\n");}

    void setViewport(int l, int b, int r, int t) override {printf("  setViewport(%d, %d, %d, %d)\n", l, b, r, t);}
    void pushModelTransform(Transform& transform) override {printf("  pushModelTransform(%s)\n", transform.getString().c_str());}
    void pushCameraTransform(Transform& transform) override {printf("  pushCameraTransform(%s)\n", transform.getString().c_str());}

    void setColor(float red, float green, float blue) override {printf("  setColor(%f, %f, %f)\n", red, green, blue);}
    void drawSprite() override {printf("  drawSprite()\n");}

    void popModelTransform() override {printf("  popModelTransform()\n");}
    void popCameraTransform() override {printf("  popCameraTransform()\n");}
};

#endif
