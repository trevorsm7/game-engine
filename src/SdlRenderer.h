#ifndef __SDLRENDERER_H__
#define __SDLRENDERER_H__

#include "IRenderer.h"
#include "SdlTexture.h"
#include "ResourceManager.h"

#include <cstdint>
#include <SDL2/SDL.h>

class SdlRenderer : public IRenderer
{
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
    ResourceManager& m_resources;
    Transform m_model;
    Transform m_camera;
    int m_width, m_height;
    struct {uint8_t r, g, b;} m_color;

public:
    SdlRenderer(SDL_Window* window, ResourceManager& resources): m_window(window), m_renderer(nullptr), m_resources(resources) {}
    ~SdlRenderer() override
    {
        if (m_renderer)
            SDL_DestroyRenderer(m_renderer);
    }

    bool init() override;

    void getSize(int& width, int& height) {SDL_GetRendererOutputSize(m_renderer, &width, &height);}

    void preRender() override;
    void postRender() override;

    void pushModelTransform(Transform& transform) override {m_model = transform;}
    void pushCameraTransform(Transform& transform) override {m_camera = transform;}

    void setColor(float red, float green, float blue) override;
    void drawSprite(const std::string& name) override;
    void drawTiles(const TileIndex& index, const TileMap& tiles) override;

    void popModelTransform() override {}
    void popCameraTransform() override {}
};

#endif
