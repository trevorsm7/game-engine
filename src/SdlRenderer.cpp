#include "SdlRenderer.h"
#include "SdlTexture.h"

bool SdlRenderer::init()
{
    // Create the 2D renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer)
    {
        fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

void SdlRenderer::preRender()
{
    SDL_RenderClear(m_renderer);
    SDL_GetRendererOutputSize(m_renderer, &m_width, &m_height);
}

void SdlRenderer::postRender()
{
    SDL_RenderPresent(m_renderer);
}

void SdlRenderer::setColor(float red, float green, float blue)
{
    // Convert/cast to 8-bit int
    m_color.r = red * 255.f;
    m_color.g = green * 255.f;
    m_color.b = blue * 255.f;
}

void SdlRenderer::drawSprite(const std::string& name)
{
    // Get texture resource
    SdlTexturePtr texture = SdlTexture::loadTexture(m_resources, m_renderer, name);
    if (!texture)
        return; // TODO: we should at least have a placeholder instead of null; assert here?

    // Apply the render color to the texture
    SDL_SetTextureColorMod(texture->getPtr(), m_color.r, m_color.g, m_color.b);

    // Set the destination rect where we will draw the texture
    // NOTE: translating origin from lower-left to upper-left
    SDL_Rect target;
    const float scaleW = m_width / m_camera.getW();
    const float scaleH = m_height / m_camera.getH();
    target.w = m_model.getW() * scaleW;
    target.h = m_model.getH() * scaleH;
    target.x = (m_model.getX() - m_camera.getX()) * scaleW;
    target.y = m_height - (m_model.getY() - m_camera.getY()) * scaleH - target.h;

    // Draw the texture
    SDL_RenderCopy(m_renderer, texture->getPtr(), nullptr, &target);
}

void SdlRenderer::drawTiles(const TileIndex& index, const TileMap& tiles)
{
    // Get texture resource
    SdlTexturePtr texture = SdlTexture::loadTexture(m_resources, m_renderer, index.name);
    if (!texture)
        return; // TODO: we should at least have a placeholder instead of null; assert here?

    // Apply the render color to the texture
    //SDL_SetTextureColorMod(texture->getPtr(), m_color.r, m_color.g, m_color.b);

    // Set the source rect from which we will draw the texture
    SDL_Rect source;
    source.w = texture->getWidth() / index.w;
    source.h = texture->getHeight() / index.h;

    // Compute the scale from camera to screen
    const float scaleW = m_width / m_camera.getW();
    const float scaleH = m_height / m_camera.getH();

    // Compute the scale from index to tile (i.e. the size in pixels as float)
    const float floatW = m_model.getW() * scaleW / tiles.w;
    const float floatH = m_model.getH() * scaleH / tiles.h;

    // Use the top-left corner of the tilemap as the origin
    const int originX = (m_model.getX() - m_camera.getX()) * scaleW;
    const int originY = m_height - (m_model.getY() - m_camera.getY() + m_model.getH()) * scaleH;

    // Set the destination rect where we will draw the texture
    SDL_Rect target;
    target.w = floatW;
    target.h = floatH;

    // Iterate over tile (x, y) indices
    int i = 0;
    for (int y = 0; y < tiles.h; ++y)
    {
        for (int x = 0; x < tiles.w; ++x)
        {
            const int tile = tiles.tiles[i++];//y * tiles.w + x];

            // Index tiles from top-left
            source.x = (tile % index.w) * source.w;
            source.y = (tile / index.w) * source.h;

            // Draw tilemap from top-left
            target.x = originX + int(x * floatW);
            target.y = originY + int(y * floatH);

            // Draw the texture
            SDL_RenderCopy(m_renderer, texture->getPtr(), &source, &target);
        }
    }
}
