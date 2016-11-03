#pragma once

#include "IRenderer.hpp"
#include "ResourceManager.hpp"

#include <cstdint>

struct SDL_Window;
struct SDL_Texture;
struct SDL_Renderer;

class SdlTexture;
typedef std::shared_ptr<SdlTexture> SdlTexturePtr;

// TODO make private to SdlRenderer? Can differentiate SDL GL textures and SDL 2D textures this way
class SdlTexture : public IResource
{
    SDL_Texture* m_texture;
    int m_width, m_height;

protected:
    SdlTexture(SDL_Texture* texture, int width, int height): m_texture(texture), m_width(width), m_height(height) {}

public:
    ~SdlTexture() override;

    SDL_Texture* getPtr() {return m_texture;}
    int getWidth() const {return m_width;}
    int getHeight() const {return m_height;}

    static SdlTexturePtr loadTexture(ResourceManager& manager, SDL_Renderer* renderer, std::string filename);

protected:
    static SDL_Texture* createTexture(SDL_Renderer* renderer, int width, int height, const void* data, int pitch, uint32_t format);

private:
    static SdlTexturePtr loadTGA(SDL_Renderer* renderer, std::vector<char>& data);

    static SdlTexturePtr getPlaceholder(SDL_Renderer* renderer);
    static SdlTexturePtr m_placeholder;
};

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
    SdlRenderer(ResourceManager& resources): m_window(nullptr), m_renderer(nullptr), m_resources(resources) {}
    ~SdlRenderer() override;

    bool init(int width, int height);

    void preRender() override;
    void postRender() override;

    void pushModelTransform(Transform& transform) override {m_model = transform;}
    void pushCameraTransform(Transform& transform) override {m_camera = transform;}

    void setColor(float red, float green, float blue) override;
    void drawSprite(const std::string& name) override;
    void drawTiles(TileMap* tilemap) override;
    void drawLines(const std::vector<float>& points) override;

    void popModelTransform() override {}
    void popCameraTransform() override {}
};
