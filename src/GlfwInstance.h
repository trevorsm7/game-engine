#ifndef __GLFWINSTANCE_H__
#define __GLFWINSTANCE_H__

#include "IRenderer.h"
#include "Scene.h"

#include <map>
#include <memory>

#define GLFW_INCLUDE_GLCOREARB
#include "GLFW/glfw3.h"

class GlfwInstance
{
    typedef std::unique_ptr<IRenderer> IRendererPtr;
    typedef std::unique_ptr<Scene> ScenePtr;

    IRendererPtr m_renderer;
    ScenePtr m_scene;
    GLFWwindow* m_window;
    std::map<int, const char*> m_keymap;

public:
    ~GlfwInstance();

    static void run(const char* script);

private:
    GlfwInstance(): m_window(nullptr) {}
    bool init(const char *script);
    bool update(double elapsedTime);

    static void callback_error(int error, const char* description);
    static void callback_key(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void callback_mouse_button(GLFWwindow* window, int button, int action, int mods);
    static void callback_window(GLFWwindow* window, int width, int height);
};

#endif
