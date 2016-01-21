#include "GlfwInstance.h"
#include "GlfwRenderer.h"
#include "DebugRenderer.h"
#include "Event.h"

#include <cmath>
#include <cstdio>

GlfwInstance::~GlfwInstance()
{
    // NOTE: glfw should destroy window automatically in glfwTerminate
    //if (m_window)
    //    glfwDestroyWindow(m_window);

    glfwTerminate();
}

void GlfwInstance::run(const char *script)
{
    GlfwInstance instance;

    if (!instance.init(script))
        return;

    double lastTime = glfwGetTime();
    double elapsedTime = 0.; // first update has zero elapsed time
    while (instance.update(elapsedTime))
    {
        // Compute time since last update
        double currentTime = glfwGetTime();
        elapsedTime = currentTime - lastTime;
        lastTime = currentTime;
    };
}

bool GlfwInstance::init(const char *script)
{
    // Populate keymap with int to string associations
    // TODO: more keys to add
    // NOTE: the compiler knows the size of the array in the for loop;
    // no need to manually track the size or use a sentinel
    static const struct {const int key; const char* const name;} keymap[] =
    {
        {GLFW_KEY_SPACE, KeyEvent::key_space},//KeyEvent::key_space},
        {GLFW_KEY_APOSTROPHE, KeyEvent::key_apostrophe},
        {GLFW_KEY_COMMA, KeyEvent::key_comma},
        {GLFW_KEY_MINUS, KeyEvent::key_minus},
        {GLFW_KEY_PERIOD, KeyEvent::key_period},
        {GLFW_KEY_SLASH, KeyEvent::key_slash},
        {GLFW_KEY_0, KeyEvent::key_0},
        {GLFW_KEY_1, KeyEvent::key_1},
        {GLFW_KEY_2, KeyEvent::key_2},
        {GLFW_KEY_3, KeyEvent::key_3},
        {GLFW_KEY_4, KeyEvent::key_4},
        {GLFW_KEY_5, KeyEvent::key_5},
        {GLFW_KEY_6, KeyEvent::key_6},
        {GLFW_KEY_7, KeyEvent::key_7},
        {GLFW_KEY_8, KeyEvent::key_8},
        {GLFW_KEY_9, KeyEvent::key_9},
        {GLFW_KEY_SEMICOLON, KeyEvent::key_semicolon},
        {GLFW_KEY_EQUAL, KeyEvent::key_equal},
        {GLFW_KEY_A, KeyEvent::key_a},
        {GLFW_KEY_B, KeyEvent::key_b},
        {GLFW_KEY_C, KeyEvent::key_c},
        {GLFW_KEY_D, KeyEvent::key_d},
        {GLFW_KEY_E, KeyEvent::key_e},
        {GLFW_KEY_F, KeyEvent::key_f},
        {GLFW_KEY_G, KeyEvent::key_g},
        {GLFW_KEY_H, KeyEvent::key_h},
        {GLFW_KEY_I, KeyEvent::key_i},
        {GLFW_KEY_J, KeyEvent::key_j},
        {GLFW_KEY_K, KeyEvent::key_k},
        {GLFW_KEY_L, KeyEvent::key_l},
        {GLFW_KEY_M, KeyEvent::key_m},
        {GLFW_KEY_N, KeyEvent::key_n},
        {GLFW_KEY_O, KeyEvent::key_o},
        {GLFW_KEY_P, KeyEvent::key_p},
        {GLFW_KEY_Q, KeyEvent::key_q},
        {GLFW_KEY_R, KeyEvent::key_r},
        {GLFW_KEY_S, KeyEvent::key_s},
        {GLFW_KEY_T, KeyEvent::key_t},
        {GLFW_KEY_U, KeyEvent::key_u},
        {GLFW_KEY_V, KeyEvent::key_v},
        {GLFW_KEY_W, KeyEvent::key_w},
        {GLFW_KEY_X, KeyEvent::key_x},
        {GLFW_KEY_Y, KeyEvent::key_y},
        {GLFW_KEY_Z, KeyEvent::key_z},
        {GLFW_KEY_LEFT_BRACKET, KeyEvent::key_leftbracket},
        {GLFW_KEY_BACKSLASH, KeyEvent::key_backslash},
        {GLFW_KEY_RIGHT_BRACKET, KeyEvent::key_rightbracket},
        {GLFW_KEY_GRAVE_ACCENT, KeyEvent::key_grave},
        {GLFW_KEY_ESCAPE, KeyEvent::key_escape},
        {GLFW_KEY_ENTER, KeyEvent::key_enter},
        {GLFW_KEY_TAB, KeyEvent::key_tab},
        {GLFW_KEY_BACKSPACE, KeyEvent::key_backspace},
        {GLFW_KEY_INSERT, KeyEvent::key_insert},
        {GLFW_KEY_DELETE, KeyEvent::key_delete},
        {GLFW_KEY_RIGHT, KeyEvent::key_right},
        {GLFW_KEY_LEFT, KeyEvent::key_left},
        {GLFW_KEY_UP, KeyEvent::key_up},
        {GLFW_KEY_DOWN, KeyEvent::key_down},
    };
    for (auto map : keymap)
        m_keymap[map.key] = map.name;

    // Init GLFW
    glfwSetErrorCallback(callback_error);
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to init GLFW\n");
        return false;
    }

    // Use OpenGL Core v4.1
    // TODO: other window hints?
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    // TODO: where to get initial window size/fullscreen from?
    m_window = glfwCreateWindow(640, 480, "Engine", NULL, NULL);
    if (m_window == nullptr)
    {
        fprintf(stderr, "Failed to create window\n");
        return false;
    }
    glfwSetWindowUserPointer(m_window, this);

    // NOTE: creating window first, then scene can change size if it wants
    // NOTE: should we be able to call quit() from load?
    // NOTE: should quit bail immediately?
    // ...current implementation doesn't take effect until end of game loop
    m_scene = std::unique_ptr<Scene>(new Scene());
    m_scene->setQuitCallback([&]{glfwSetWindowShouldClose(m_window, GL_TRUE);});
    m_scene->load(script);
    // TODO: send an initial "resize" notification to scene?
    int width, height;
    glfwGetWindowSize(m_window, &width, &height);
    m_scene->resize(width, height);

    // Set GLFW callbacks
    glfwSetKeyCallback(m_window, callback_key);
    //glfwSetCursorPosCallback(m_window, callback_mouse_move);
    glfwSetMouseButtonCallback(m_window, callback_mouse_button);
    //glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetWindowSizeCallback(m_window, callback_window);
    //glfwSetFramebufferSizeCallback(m_window, resizeCallback);

    // NOTE: should context handling move into GlfwRenderer?
    // only if multiple windows in one thread
    // NOTE: this must be called before glfwSwapInterval
    glfwMakeContextCurrent(m_window);

    // Force vsync on current context
    glfwSwapInterval(1);

    //m_renderer = std::unique_ptr<IRenderer>(new DebugRenderer());
    m_renderer = std::unique_ptr<IRenderer>(new GlfwRenderer(m_window));
    m_renderer->init();

    return true;
}

bool GlfwInstance::update(double elapsedTime)
{
    // Try to process events first
    glfwPollEvents();

    // Send elapsed time down to game objects
    m_scene->update(elapsedTime);

    // Render last after input and updates
    m_renderer->preRender();
    m_scene->render(m_renderer.get());
    m_renderer->postRender();

    // TODO: support loading/saving Scenes from script

    return !glfwWindowShouldClose(m_window);
}

void GlfwInstance::callback_error(int error, const char* description)
{
    fprintf(stderr, "%s\n", description);
}

void GlfwInstance::callback_key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // TODO: provide functionality for reporting the next key pressed?
    // like for if we want to assign a key to the next key pressed

    if (action == GLFW_PRESS || action == GLFW_RELEASE)
    {
        void *ptr = glfwGetWindowUserPointer(window);
        auto instance = reinterpret_cast<GlfwInstance*>(ptr);

        if (instance && instance->m_scene)
        {
            auto keyIt = instance->m_keymap.find(key);
            if (keyIt != instance->m_keymap.end())
            {
                KeyEvent event;
                event.name = keyIt->second;
                event.down = (action == GLFW_PRESS);

                if (instance->m_scene->keyEvent(event))
                    return;
            }
        }

        // If key is unhandled, provide default behavior for escape
        // TODO: provide other default behaviors?
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void GlfwInstance::callback_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
    // TODO: support other mouse buttons
    // TODO: support mouse drag while a button is down
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        void *ptr = glfwGetWindowUserPointer(window);
        auto instance = reinterpret_cast<GlfwInstance*>(ptr);

        if (instance && instance->m_scene)
        {
            double x, y;
            int width, height;
            glfwGetCursorPos(window, &x, &y);
            glfwGetWindowSize(window, &width, &height);

            MouseEvent event;
            event.x = floor(x);
            event.y = height - floor(y) - 1;
            event.w = width;
            event.h = height;
            event.down = (action == GLFW_PRESS);
            //printf("mouse %s at (%d, %d)\n", event.down ? "down" : "up", event.x, event.y);

            instance->m_scene->mouseEvent(event);
            // NOTE: do we want a return value for this?
            //if (instance->m_scene->keyEvent(event))
            //    return;
        }
    }
}

void GlfwInstance::callback_window(GLFWwindow* window, int width, int height)
{
    void *ptr = glfwGetWindowUserPointer(window);
    auto instance = reinterpret_cast<GlfwInstance*>(ptr);

    if (instance && instance->m_scene)
        instance->m_scene->resize(width, height);
}
