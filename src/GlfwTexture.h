#ifndef __GLFWTEXTURE_H__
#define __GLFWTEXTURE_H__

#include "IResource.h"

#define GLFW_INCLUDE_GLCOREARB
#include "GLFW/glfw3.h"

class GlfwTexture : public IResource
{
private:
    GLuint m_texture;
    // TODO: may want to save width/height in case we use pixel texture coords

protected:
    GlfwTexture(GLuint texture): m_texture(texture) {}

public:
    ~GlfwTexture() override {glDeleteTextures(1, &m_texture);}

    void bind() {glBindTexture(GL_TEXTURE_2D, m_texture);}

    // TODO: consider reworking createTexture as an IResourcePtr loader
    static GLuint createTexture(GLsizei width, GLsizei height, const GLvoid* data, GLenum channels, GLenum order, GLenum format, bool useMipmap);
    static IResourcePtr tgaLoader(std::string filename);
};

#endif
