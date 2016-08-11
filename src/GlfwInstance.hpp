#pragma once

#include "IRenderer.hpp"
#include "ResourceManager.hpp"
#include "Scene.hpp"
#include "GlfwGamepad.hpp"

#include <map>
#include <memory>

#define GLFW_INCLUDE_GLCOREARB
#include "GLFW/glfw3.h"

class GlfwInstance
{
    typedef std::unique_ptr<IRenderer> IRendererPtr;
    typedef std::unique_ptr<Scene> ScenePtr;

    ResourceManager m_resources;
    IRendererPtr m_renderer;
    ScenePtr m_scene;
    GLFWwindow* m_window;
    std::vector<GlfwGamepad> m_gamepads;
    std::map<int, const char*> m_keymap;

public:
    ~GlfwInstance();

    static void run(const char* script);

private:
    GlfwInstance(): m_window(nullptr) {}

    bool init(const char *script);
    void pollEvents();
    void update(double elapsedTime);
    void render();

    bool isQuit() const {return glfwWindowShouldClose(m_window);}

    static void callback_error(int error, const char* description);
    static void callback_key(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void callback_mouse_button(GLFWwindow* window, int button, int action, int mods);
    static void callback_window(GLFWwindow* window, int width, int height);
};
