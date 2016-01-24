#ifndef __GLFWRENDERER_H__
#define __GLFWRENDERER_H__

#include "IRenderer.h"

#include <vector>

#define GLFW_INCLUDE_GLCOREARB
#include "GLFW/glfw3.h"

class GlfwRenderer : public IRenderer
{
    GLFWwindow* m_window;
    GLuint m_spriteVAO;
    GLint m_modelScale;
    GLint m_modelOffset;
    GLint m_cameraScale;
    GLint m_cameraOffset;
    GLint m_color;
    //std::vector<Matrix4> m_modelStack;
    //std::vector<Matrix4> m_viewStack;

public:
    GlfwRenderer(GLFWwindow* window): m_window(window) {}
    ~GlfwRenderer() override {}

    void init() override;
    void preRender() override;
    void postRender() override;

    void setViewport(int left, int bottom, int right, int top) override;
    void pushModelTransform(Transform& transform) override;
    void pushCameraTransform(Transform& transform) override;

    void setColor(float red, float green, float blue) override;
    void drawSprite() override;

    void popModelTransform() override;
    void popCameraTransform() override;

private:
    GLuint loadShader(const char* shaderCode, GLenum shaderType);
};

#endif
