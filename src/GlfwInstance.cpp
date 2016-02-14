#include "GlfwInstance.h"
#include "GlfwRenderer.h"
#include "DebugRenderer.h"
#include "Event.h"

#include <cmath>
#include <cstdio>

GlfwInstance::~GlfwInstance()
{
    glfwTerminate();
}

void GlfwInstance::run(const char* script)
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

bool GlfwInstance::init(const char* script)
{
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
    m_scene = ScenePtr(new Scene());
    m_scene->setQuitCallback([&]{glfwSetWindowShouldClose(m_window, GL_TRUE);});
    m_scene->setRegisterControlCallback([&](const char* action)->bool
    {
        static const struct {const int key; const char* const name;} keymap[] =
        {
            // Map GLFW keys to internal string names
            {GLFW_KEY_SPACE, "action"},
            {GLFW_KEY_W, "w"},
            {GLFW_KEY_A, "a"},
            {GLFW_KEY_S, "s"},
            {GLFW_KEY_D, "d"},
            {GLFW_KEY_ESCAPE, "quit"},
            {GLFW_KEY_RIGHT, "right"},
            {GLFW_KEY_LEFT, "left"},
            {GLFW_KEY_UP, "up"},
            {GLFW_KEY_DOWN, "down"},
        };

        // TODO: replace with a mechanism for default bindings and loading/saving custom bindings
        for (auto map : keymap)
        {
            if (strcmp(action, map.name) == 0)
            {
                m_keymap[map.key] = map.name;
                return true;
            }
        }

        return false;
    });
    m_scene->load(script);

    // Send an initial resize notification to scene
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
    // NOTE: this must be called before glfwSwapInterval
    glfwMakeContextCurrent(m_window);

    // Force vsync on current context
    glfwSwapInterval(1);

    //m_renderer = IRendererPtr(new DebugRenderer());
    m_renderer = IRendererPtr(new GlfwRenderer(m_window));
    m_renderer->init();

    return true;
}

bool GlfwInstance::update(double elapsedTime)
{
    // Try to process events first
    glfwPollEvents();

    // NOTE: joystick functions not affected by glfwPollEvents
    static int wasPresent = 0;
    int present = glfwJoystickPresent(GLFW_JOYSTICK_1);
    if (present != wasPresent)
    {
        const char* name = glfwGetJoystickName(GLFW_JOYSTICK_1);
        printf("%s: %sconnected\n", name, present ? "" : "dis");
        wasPresent = present;
    }

    if (present)
    {
        int nAxes, nButtons;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &nAxes);
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &nButtons);
        static char up = 0, right = 0, down = 0, left = 0, space = 0;
        if (buttons[14] != up)
        {
            callback_key(m_window, GLFW_KEY_UP, 0, buttons[14] ? GLFW_PRESS : GLFW_RELEASE, 0);
            //callback_key(m_window, GLFW_KEY_W, 0, buttons[14] ? GLFW_PRESS : GLFW_RELEASE, 0);
            up = buttons[14];
        }
        if (buttons[15] != right)
        {
            callback_key(m_window, GLFW_KEY_RIGHT, 0, buttons[15] ? GLFW_PRESS : GLFW_RELEASE, 0);
            //callback_key(m_window, GLFW_KEY_D, 0, buttons[15] ? GLFW_PRESS : GLFW_RELEASE, 0);
            right = buttons[15];
        }
        if (buttons[16] != down)
        {
            callback_key(m_window, GLFW_KEY_DOWN, 0, buttons[16] ? GLFW_PRESS : GLFW_RELEASE, 0);
            //callback_key(m_window, GLFW_KEY_S, 0, buttons[16] ? GLFW_PRESS : GLFW_RELEASE, 0);
            down = buttons[16];
        }
        if (buttons[17] != left)
        {
            callback_key(m_window, GLFW_KEY_LEFT, 0, buttons[17] ? GLFW_PRESS : GLFW_RELEASE, 0);
            //callback_key(m_window, GLFW_KEY_A, 0, buttons[17] ? GLFW_PRESS : GLFW_RELEASE, 0);
            left = buttons[17];
        }
        if (buttons[1] != space)
        {
            callback_key(m_window, GLFW_KEY_SPACE, 0, buttons[1] ? GLFW_PRESS : GLFW_RELEASE, 0);
            space = buttons[1];
        }
    }

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
        void* ptr = glfwGetWindowUserPointer(window);
        auto instance = reinterpret_cast<GlfwInstance*>(ptr);

        if (instance && instance->m_scene)
        {
            auto keyIt = instance->m_keymap.find(key);
            if (keyIt != instance->m_keymap.end())
            {
                ControlEvent event;
                event.name = keyIt->second;
                event.down = (action == GLFW_PRESS);

                if (instance->m_scene->controlEvent(event))
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
        void* ptr = glfwGetWindowUserPointer(window);
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
    void* ptr = glfwGetWindowUserPointer(window);
    auto instance = reinterpret_cast<GlfwInstance*>(ptr);

    if (instance && instance->m_scene)
        instance->m_scene->resize(width, height);
}
