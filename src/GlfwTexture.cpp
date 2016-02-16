#include "GlfwTexture.h"

#include <fstream>
#include <cstdio>

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

IResourcePtr GlfwTexture::tgaLoader(std::string filename)
{
    // Open the file
    std::fstream file;
    file.open(filename, file.in | file.binary);
    if (!file.is_open())
    {
        fprintf(stderr, "Unable to open file \"%s\"\n", filename.c_str());
        return nullptr;
    }

    // Get the length of the file
    file.seekg(0, file.end);
    int length = file.tellg();
    file.seekg(0, file.beg);

    // Fail if file is smaller than header size
    if (length < sizeof(TGAHeader))
    {
        fprintf(stderr, "File size too short for header in \"%s\"\n", filename.c_str());
        return nullptr;
    }

    // Read TGA header
    TGAHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(TGAHeader));
    length -= sizeof(TGAHeader);

    GLsizei width = header.imageSpec.imageWidth;
    GLsizei height = header.imageSpec.imageHeight;
    GLsizei size = width * height;

    // Validate binary format
    GLenum channels, order, format;
    if (header.imageSpec.pixelDepth == 8 && header.imageType == 3)
    {
        channels = GL_RED;
        order = GL_RED;
        format = GL_UNSIGNED_BYTE;
    }
    else if (header.imageSpec.pixelDepth == 16 && header.imageType == 2)
    {
        size *= 2; // 2 bytes per pixel
        channels = GL_RGB;
        order = GL_BGRA;
        format = GL_UNSIGNED_SHORT_1_5_5_5_REV;
    }
    else if (header.imageSpec.pixelDepth == 24 && header.imageType == 2)
    {
        size *= 3; // 3 bytes per pixel
        channels = GL_RGB;
        order = GL_BGR;
        format = GL_UNSIGNED_BYTE;
    }
    else
    {
        fprintf(stderr, "Unsupported TGA format in \"%s\"\n", filename.c_str());
        return nullptr;
    }

    // Fail if file is smaller than data size
    if (length < size)
    {
        fprintf(stderr, "File size too short for data in \"%s\"\n", filename.c_str());
        return nullptr;
    }

    // Read the image data
    auto data = std::unique_ptr<char>(new char [size]);
    file.read(data.get(), size);
    file.close();

    // Create an OpenGL texture with the data and return GlfwTexture ptr
    // NOTE: we probably don't want to use mipmapping with sprites?
    GLuint texture = createTexture(width, height, data.get(), channels, order, format, false);
    return IResourcePtr(new GlfwTexture(texture));
}
