#ifndef __GLFWRENDERER_H__
#define __GLFWRENDERER_H__

#include "IRenderer.h"
#include "ResourceManager.h"

#include <vector>

#define GLFW_INCLUDE_GLCOREARB
#include "GLFW/glfw3.h"

class GlfwRenderer : public IRenderer
{
    GLFWwindow* m_window;
    ResourceManager& m_resources;
    Transform m_model;
    GLuint m_spriteVAO;
    GLint m_modelScale;
    GLint m_modelOffset;
    GLint m_cameraScale;
    GLint m_cameraOffset;
    GLint m_textureScale;
    GLint m_textureOffset;
    GLint m_color;
    //std::vector<Matrix4> m_modelStack;
    //std::vector<Matrix4> m_viewStack;

public:
    GlfwRenderer(GLFWwindow* window, ResourceManager& resources): m_window(window), m_resources(resources) {}
    ~GlfwRenderer() override {} // TODO: cleanup buffers

    bool init() override;
    void preRender() override;
    void postRender() override;

    void pushModelTransform(Transform& transform) override;
    void pushCameraTransform(Transform& transform) override;

    void setColor(float red, float green, float blue) override;
    void drawSprite(const std::string& name) override;
    void drawTiles(TileMap* tilemap) override;

    void popModelTransform() override;
    void popCameraTransform() override;

private:
    GLuint loadShader(const char* shaderCode, GLenum shaderType);
};

#endif
