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

// TODO: change args from float to int (in pixels)?
void SdlRenderer::drawSprite(const std::string& name)
{
    // Get texture resource
    SdlTexturePtr texture = SdlTexture::loadTexture(m_resources, m_renderer, name);
    if (!texture)
        return; // TODO: we should at least have a placeholder instead of null; assert here?

    // Apply the render color to the texture
    SDL_SetTextureColorMod(texture->getPtr(), m_color.r, m_color.g, m_color.b);

    // Set the source rect from which we will draw the texture
    // NOTE: translating origin from lower-left to upper-left
    /*SDL_Rect source;
    int texWidth, texHeight;
    SDL_QueryTexture(texture->getPtr(), nullptr, nullptr, &texWidth, &texHeight);
    source.w = texWidth;
    source.h = texHeight;
    source.x = 0;
    source.y = texHeight - (0) - source.h;*/

    // Set the destination rect where we will draw the texture
    // NOTE: translating origin from lower-left to upper-left
    SDL_Rect target;
    float wscale = m_width / m_camera.getW();
    float hscale = m_height / m_camera.getH();
    target.w = m_model.getW() * wscale;
    target.h = m_model.getH() * hscale;
    target.x = (m_model.getX() - m_camera.getX()) * wscale;
    target.y = m_height - (m_model.getY() - m_camera.getY()) * hscale - target.h;

    // Draw the texture
    SDL_RenderCopy(m_renderer, texture->getPtr(), nullptr/*&source*/, &target);
}
