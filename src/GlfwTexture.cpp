#include "GlfwTexture.h"

#include <fstream>
#include <cstdio>

GlfwTexturePtr GlfwTexture::loadTexture(ResourceManager& manager, std::string filename)
{
    // Return the resource if it is cached
    // TODO: should we check for special case where resource is loaded, but not as a GlfwTexture?
    IResourcePtr resource = manager.getResource(filename);
    GlfwTexturePtr texture = std::dynamic_pointer_cast<GlfwTexture>(resource);
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
        texture = getPlaceholder();
        manager.bindResource(filename, texture);
        return texture;
    }

    // Pass the raw data to the loader
    texture = loadTGA(data);
    if (!texture)
    {
        fprintf(stderr, "Malformed data in file \"%s\"\n", filename.c_str());
        texture = getPlaceholder();
        manager.bindResource(filename, texture);
        return texture;
    }

    // Cache the resource and return it
    manager.bindResource(filename, texture);
    return texture;
}

GLuint GlfwTexture::createTexture(GLsizei width, GLsizei height, const GLvoid* data, GLenum channels, GLenum order, GLenum format, bool useMipmap)
{
    GLuint texture;

    // Buffer image data
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, channels, width, height, 0, order, format, data);

    // Set wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Set filtering parameters
    if (useMipmap)
    {
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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

GlfwTexturePtr GlfwTexture::loadTGA(std::vector<char>& data)
{
    GlfwTexturePtr ptr;

    // Fail if file is smaller than header size
    if (data.size() < sizeof(TGAHeader))
    {
        fprintf(stderr, "TGA file is smaller than header size\n");
        return ptr;
    }

    // Parse TGA header
    TGAHeader* header = reinterpret_cast<TGAHeader*>(data.data());
    GLsizei width = header->imageSpec.imageWidth;
    GLsizei height = header->imageSpec.imageHeight;
    GLsizei size = width * height;

    // Validate binary format
    GLenum channels, order, format;
    if (header->imageSpec.pixelDepth == 8 && header->imageType == 3)
    {
        channels = GL_RED;
        order = GL_RED;
        format = GL_UNSIGNED_BYTE;
    }
    else if (header->imageSpec.pixelDepth == 16 && header->imageType == 2)
    {
        size *= 2; // 2 bytes per pixel
        channels = GL_RGB;
        order = GL_BGRA;
        format = GL_UNSIGNED_SHORT_1_5_5_5_REV;
    }
    else if (header->imageSpec.pixelDepth == 24 && header->imageType == 2)
    {
        size *= 3; // 3 bytes per pixel
        channels = GL_RGB;
        order = GL_BGR;
        format = GL_UNSIGNED_BYTE;
    }
    else
    {
        fprintf(stderr, "Unsupported TGA format\n");
        return ptr;
    }

    // Fail if file is smaller than data size
    if (data.size() < sizeof(TGAHeader) + size)
    {
        fprintf(stderr, "TGA file is smaller than data size\n");
        return ptr;
    }

    // Create an OpenGL texture with the data and return GlfwTexture ptr
    // NOTE: we probably don't want to use mipmapping with sprites?
    GLuint texture = createTexture(width, height, &data[sizeof(TGAHeader)], channels, order, format, false);
    ptr = GlfwTexturePtr(new GlfwTexture(texture));
    return ptr;
}

GlfwTexturePtr GlfwTexture::m_placeholder;

GlfwTexturePtr GlfwTexture::getPlaceholder()
{
    // TODO: need to block here if we want to support multi-threaded access
    if (m_placeholder)
        return m_placeholder;

    // Generate a checkered pattern for missing textures
    struct {uint8_t b, g, r;} __attribute__((__packed__)) checkered[16];
    for (int i = 0; i < 16; ++i)
    {
        // Alternate coloring evens or odds each row
        if (i % 2 != (i >> 2) % 2)
            checkered[i] = {255, 255, 255};
        else
            checkered[i] = {0, 0, 0};
    }

    // Create a texture with the data and return it
    GLuint texture = createTexture(4, 4, checkered, GL_RGB, GL_BGR, GL_UNSIGNED_BYTE, false);
    m_placeholder = GlfwTexturePtr(new GlfwTexture(texture));
    return m_placeholder;
}
