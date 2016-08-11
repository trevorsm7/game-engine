#include "SdlTexture.hpp"

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

struct TGAColorMapSpec
{
    uint16_t offsetBytes;
    uint16_t entryCount;
    uint8_t entrySize;
}
__attribute__((__packed__));

struct TGAImageSpec
{
    uint16_t xOrigin; // 0?
    uint16_t yOrigin; // 0?
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint8_t pixelDepth; // 8, 16, 24, etc
    uint8_t descriptor; // [..vhaaaa] vertical/horizontal flip, alpha bits
}
__attribute__((__packed__));

struct TGAHeader
{
    uint8_t idLength; // 0
    uint8_t colorMapType; // 0
    uint8_t imageType; // 2 for color, 3 for grey
    TGAColorMapSpec colorMapSpec; // 0, 0, 0
    TGAImageSpec imageSpec;
}
__attribute__((__packed__));

struct TGAData24 {uint8_t b, g, r;} __attribute__((__packed__));
struct TGAData16 {uint16_t b:5, g:5, r:5, a:1;} __attribute__((__packed__));
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
    struct {uint8_t b, g, r;} __attribute__((__packed__)) checkered[size];
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
