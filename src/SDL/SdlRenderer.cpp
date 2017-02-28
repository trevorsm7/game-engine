#include "SdlRenderer.hpp"
#include "TileMap.hpp"

#include "SDL.h"

SdlRenderer::~SdlRenderer()
{
    if (m_renderer)
        SDL_DestroyRenderer(m_renderer);

    if (m_window)
        SDL_DestroyWindow(m_window);
}

bool SdlRenderer::init(int width, int height)
{
    // Create the window
    m_window = SDL_CreateWindow("Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_window)
    {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return false;
    }

    // Create the 2D renderer
    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer)
    {
        fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

/*bool SdlGlRenderer::init(int width, int height)
{
    //SDL_GLContext m_context;

    // Set OpenGL context attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create the window
    m_window = SDL_CreateWindow("Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    //m_window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!m_window)
    {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return false;
    }

    //SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP); // 0, SDL_WINDOW_FULLSCREEN

    // Create the OpenGL context
    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context)
    {
        fprintf(stderr, "Failed to create GL context: %s\n", SDL_GetError());
        return false;
    }

    // Set vertical retrace synchronization
    SDL_GL_SetSwapInterval(1);

    //int fbWidth, fbHeight;
    //SDL_GL_GetDrawableSize(m_window, &fbWidth, &fbHeight);
    //fprintf(stderr, "Framebuffer size: %d, %d\n", fbWidth, fbHeight);

    //if (m_context)
    //    SDL_GL_DeleteContext(m_context);
}*/

void SdlRenderer::preRender()
{
    //glClearColor(0.0, 0.0, 0.0, 1.0);
    //glClear(GL_COLOR_BUFFER_BIT);
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
    SDL_GetRendererOutputSize(m_renderer, &m_width, &m_height);
}

void SdlRenderer::postRender()
{
    //SDL_GL_SwapWindow(m_window);
    SDL_RenderPresent(m_renderer);
}

void SdlRenderer::setColor(float red, float green, float blue)
{
    // Convert/cast to 8-bit int
	//std::clamp(red, 0.f, 255.f); // C++17
    m_color.r = uint8_t(std::min(std::max(red * 255.f, 0.f), 255.f));
	m_color.g = uint8_t(std::min(std::max(green * 255.f, 0.f), 255.f));
	m_color.b = uint8_t(std::min(std::max(blue * 255.f, 0.f), 255.f));
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
    SDL_Rect target;
    const float scaleW = m_width / m_camera.getScaleX();
    const float scaleH = m_height / m_camera.getScaleY();
    target.w = int(ceil(m_model.getScaleX() * scaleW));
    target.h = int(ceil(m_model.getScaleY() * scaleH));
    target.x = int(floor((m_model.getX() - m_camera.getX()) * scaleW));
    target.y = int(floor((m_model.getY() - m_camera.getY()) * scaleH));

    // Draw the texture
    SDL_RenderCopy(m_renderer, texture->getPtr(), nullptr, &target);
}

void SdlRenderer::drawTiles(const TileMap* tilemap)
{
    assert(tilemap != nullptr);
    const TileSet* tileset = tilemap->getTileSet();
    if (!tileset)
        return;

    // Get texture resource
    SdlTexturePtr texture = SdlTexture::loadTexture(m_resources, m_renderer, tileset->getFilename());
    if (!texture)
        return; // TODO: we should at least have a placeholder instead of null; assert here?

    const TileMask* tileMask = tilemap->getTileMask();

    // Apply the render color to the texture
    SDL_SetTextureColorMod(texture->getPtr(), m_color.r, m_color.g, m_color.b);

    // Set the source rect from which we will draw the texture
    SDL_Rect source;
    source.w = texture->getWidth() / tileset->getCols();
    source.h = texture->getHeight() / tileset->getRows();

    // Compute the scale from camera to screen
    const float scaleW = m_width / m_camera.getScaleX();
    const float scaleH = m_height / m_camera.getScaleY();

    // Compute the scale from index to tile (i.e. the size in pixels as float)
    const float floatW = m_model.getScaleX() * scaleW;
    const float floatH = m_model.getScaleY() * scaleH;

    // Use the top-left corner of the tilemap as the origin
    const float originX = (m_model.getX() - m_camera.getX()) * scaleW;
    const float originY = (m_model.getY() - m_camera.getY()) * scaleH;

    // Set the destination rect where we will draw the texture
    SDL_Rect target;
    target.w = int(ceil(floatW));
    target.h = int(ceil(floatH));

    // Iterate over tile (x, y) indices
    int i = 0;
    float yf = originY;
    for (int y = 0; y < tilemap->getRows(); ++y, yf += floatH)
    {
        float xf = originX;
        for (int x = 0; x < tilemap->getCols(); ++x, xf += floatW)
        {
            // Skip if tile index invalid (blank tile)
            const int tile = tilemap->getIndex(i++);
            if (!tileset->isValidIndex(tile))
                continue;

            // Index tiles from top-left
            source.x = tileset->getIndexCol(tile) * source.w;
            source.y = tileset->getIndexRow(tile) * source.h;

            // Draw tilemap from top-left
            target.x = int(xf);
            target.y = int(yf);

            if (tileMask)
            {
                const uint8_t mask = tileMask->getMask(x, y);
                if (mask == 0)
                    continue;

                uint8_t r = uint8_t(uint16_t(m_color.r) * uint16_t(mask + 1) / 256);
                uint8_t g = uint8_t(uint16_t(m_color.g) * uint16_t(mask + 1) / 256);
                uint8_t b = uint8_t(uint16_t(m_color.b) * uint16_t(mask + 1) / 256);
                SDL_SetTextureColorMod(texture->getPtr(), r, g, b);
            }

            // Draw the texture
            SDL_RenderCopy(m_renderer, texture->getPtr(), &source, &target);
        }
    }
}

inline void mapColorScale(SDL_Renderer* renderer, float step)
{
    if (step < 1.f)
        SDL_SetRenderDrawColor(renderer, 255, uint8_t(255*step), 0, 255);
    else if (step < 2.f)
        SDL_SetRenderDrawColor(renderer, uint8_t(255*(2.f-step)), 255, 0, 255);
    else if (step < 3.f)
        SDL_SetRenderDrawColor(renderer, 0, 255, uint8_t(255*(step-2.f)), 255);
    else if (step < 4.f)
        SDL_SetRenderDrawColor(renderer, 0, uint8_t(255*(4.f-step)), 255, 255);
    else if (step < 5.f)
        SDL_SetRenderDrawColor(renderer, uint8_t(255*(step-4.f)), 0, 255, 255);
    else
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
}

void SdlRenderer::drawLines(const std::vector<float>& points)
{
    const float scaleW = m_width / m_camera.getScaleX();
    const float scaleH = m_height / m_camera.getScaleY();

    for (size_t i = 0; i < points.size(); i++)
    {
        int segments = int(points[i]);

        assert(segments >= 2);
        assert(i + segments * 2 < points.size());

        int lastX = int(floor((points[++i] - m_camera.getX()) * scaleW));
        int lastY = int(floor((points[++i] - m_camera.getY()) * scaleH));

        float stepSize = 5.f / (segments-1);
        float step = 0.f;

        while (segments-- >= 2)
        {
            int thisX = int(floor((points[++i] - m_camera.getX()) * scaleW));
            int thisY = int(floor((points[++i] - m_camera.getY()) * scaleH));
            mapColorScale(m_renderer, step); step += stepSize;
            SDL_RenderDrawLine(m_renderer, lastX, lastY, thisX, thisY);
            lastX = thisX;
            lastY = thisY;
        }
    }
}

// =============================================================================
// SdlTexture
// =============================================================================

SdlTexture::~SdlTexture()
{
    if (m_texture)
        SDL_DestroyTexture(m_texture);
}

// TODO: this is essentially identical to the coresponding function in GlfwTexture; refactor!
SdlTexturePtr SdlTexture::loadTexture(ResourceManager& manager, SDL_Renderer* renderer, std::string filename)
{
    // Return the resource if it is cached
    // TODO: should we check for special case where resource is loaded, but not as a GlfwTexture?
    IResourcePtr resource = manager.getResource(filename);
    SdlTexturePtr texture = std::dynamic_pointer_cast<SdlTexture>(resource);
    if (texture)
        return texture;

    // Get file extension
    size_t index = filename.find_last_of('.');
    std::string ext = filename.substr(index + 1);

    // TODO: determine loader function to call for given extension (return placeholder if none)

    // Load the raw file data into memory
    std::vector<char> data;
    if (!manager.loadRawData(filename, data))
    {
        texture = getPlaceholder(renderer);
        manager.bindResource(filename, texture);
        return texture;
    }

    // Pass the raw data to the loader
    texture = loadTGA(renderer, data);
    if (!texture)
    {
        fprintf(stderr, "Malformed data in file \"%s\"\n", filename.c_str());
        texture = getPlaceholder(renderer);
        manager.bindResource(filename, texture);
        return texture;
    }

    // Cache the resource and return it
    manager.bindResource(filename, texture);
    return texture;
}

SDL_Texture* SdlTexture::createTexture(SDL_Renderer* renderer, int width, int height, const void* data, int pitch, uint32_t format)
{
    SDL_Texture* texture = SDL_CreateTexture(renderer, format, SDL_TEXTUREACCESS_STATIC, width, height);
    if (!texture)
    {
        fprintf(stderr, "Failed create texture: %s\n", SDL_GetError());
        return nullptr;
    }

    if (SDL_UpdateTexture(texture, nullptr, data, pitch))
    {
        fprintf(stderr, "Failed update texture: %s\n", SDL_GetError());
        SDL_DestroyTexture(texture);
        return nullptr;
    }

    return texture;
}

#pragma pack(push,1)
struct TGAColorMapSpec
{
    uint16_t offsetBytes;
    uint16_t entryCount;
    uint8_t entrySize;
}
#pragma pack(pop)
#ifndef WIN32
__attribute__((__packed__))
#endif
;

#pragma pack(push,1)
struct TGAImageSpec
{
    uint16_t xOrigin; // 0?
    uint16_t yOrigin; // 0?
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint8_t pixelDepth; // 8, 16, 24, etc
    uint8_t descriptor; // [..vhaaaa] vertical/horizontal flip, alpha bits
}
#pragma pack(pop)
#ifndef WIN32
__attribute__((__packed__))
#endif
;

#pragma pack(push,1)
struct TGAHeader
{
    uint8_t idLength; // 0
    uint8_t colorMapType; // 0
    uint8_t imageType; // 2 for color, 3 for grey
    TGAColorMapSpec colorMapSpec; // 0, 0, 0
    TGAImageSpec imageSpec;
}
#pragma pack(pop)
#ifndef WIN32
__attribute__((__packed__))
#endif
;

#pragma pack(push,1)
struct TGAData24 {uint8_t b, g, r;}
#pragma pack(pop)
#ifndef WIN32
__attribute__((__packed__))
#endif
;

#pragma pack(push,1)
struct TGAData16 {uint16_t b:5, g:5, r:5, a:1;}
#pragma pack(pop)
#ifndef WIN32
__attribute__((__packed__))
#endif
;

typedef uint8_t TGAData8;

/*struct TGAFile
{
    TGAHeader header;
    union
    {
        TGAData24 data24[1];
        TGAData16 data16[1];
        TGAData8 data8[1];
    }
}
__attribute__((__packed__));*/

// TODO: this is also very similar to the same function in GlfwTexture; refactor?
SdlTexturePtr SdlTexture::loadTGA(SDL_Renderer* renderer, std::vector<char>& data)
{
    SdlTexturePtr ptr;

    // Fail if file is smaller than header size
    if (data.size() < sizeof(TGAHeader))
    {
        fprintf(stderr, "TGA file is smaller than header size\n");
        return ptr;
    }

    // Parse TGA header
    TGAHeader* header = reinterpret_cast<TGAHeader*>(data.data());
    int width = header->imageSpec.imageWidth;
    int height = header->imageSpec.imageHeight;

    // Validate binary format
    int bytesPerPixel;
    uint32_t format;
    if (header->imageSpec.pixelDepth == 16 && header->imageType == 2)
    {
        bytesPerPixel = 2;
        format = SDL_PIXELFORMAT_BGRA5551;
    }
    else if (header->imageSpec.pixelDepth == 24 && header->imageType == 2)
    {
        bytesPerPixel = 3;
        format = SDL_PIXELFORMAT_BGR24;
    }
    else
    {
        fprintf(stderr, "Unsupported TGA format\n");
        return ptr;
    }

    int pitch = width * bytesPerPixel;
    int size = height * pitch;

    // Fail if file is smaller than data size
    if (data.size() < sizeof(TGAHeader) + size)
    {
        fprintf(stderr, "TGA file is smaller than data size\n");
        return ptr;
    }

    char* pixels = &data[sizeof(TGAHeader)];

    // Swap rows between top and bottom to invert row order
    auto temp = std::unique_ptr<char[]>(new char [pitch]);
    for (int i = 0; i < height / 2; ++i)
    {
        char* low = &pixels[pitch * i];
        char* high = &pixels[pitch * (height - i - 1)];
        memcpy(temp.get(), low, pitch);
        memcpy(low, high, pitch);
        memcpy(high, temp.get(), pitch);
    }

    // Create an SDL texture with the data and return
    SDL_Texture* texture = createTexture(renderer, width, height, pixels, pitch, format);
    ptr = SdlTexturePtr(new SdlTexture(texture, width, height));
    return ptr;
}

SdlTexturePtr SdlTexture::m_placeholder;

SdlTexturePtr SdlTexture::getPlaceholder(SDL_Renderer* renderer)
{
    // TODO: need to block here if we want to support multi-threaded access
    if (m_placeholder)
        return m_placeholder;

    // Generate a checkered pattern for missing textures
    const int width = 4, height = 4, size = width * height;
	TGAData24 checkered[size];
    for (int i = 0; i < size; ++i)
    {
        // Alternate coloring evens or odds each row
        if (i % 2 != (i / width) % 2)
            checkered[i] = {255, 255, 255};
        else
            checkered[i] = {0, 0, 0};
    }

    // Create a texture with the data and return it
    const int pitch = width * 3;
    const uint32_t format = SDL_PIXELFORMAT_BGR24;
    SDL_Texture* texture = createTexture(renderer, width, height, checkered, pitch, format);
    m_placeholder = SdlTexturePtr(new SdlTexture(texture, width, height));
    return m_placeholder;
}
