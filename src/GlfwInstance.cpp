#include "GlfwInstance.h"
#include "GlfwRenderer.h"
#include "GlfwTexture.h"
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

    // The first event poll will take extra time; get it out of the way early
    instance.pollEvents();

    double lastTime = glfwGetTime();
    while (!instance.isQuit())
    {
        // Render first; returns right after vsync
        instance.render();

        // We could improve the input latency by sleeping after vsync to push input/update closer to next vsync

        // Process event queue
        instance.pollEvents();

        // Compute elapsed time since last update and pass to engine
        // TODO: check for overflow
        const double currentTime = glfwGetTime();
        const double elapsedTime = currentTime - lastTime;
        lastTime = currentTime;
        instance.update(elapsedTime);
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
    m_scene = ScenePtr(new Scene(m_resources));
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
    if (!m_scene->load(script))
        return false;

    // TODO default window size??
    if (m_scene->isPortraitHint())
        glfwSetWindowSize(m_window, 480, 640);

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

    // Set up gamepads
    //for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i)
    //    m_gamepads.emplace_back(i);
    m_gamepads.emplace_back(GLFW_JOYSTICK_1);
    m_gamepads.emplace_back(GLFW_JOYSTICK_2);

    // Assign some default button mappings (for PS4 controller)
    GlfwGamepad& gamepad = m_gamepads[0];
    gamepad.registerControl(1, "action");
    gamepad.registerControl(14, "up");//"w");
    gamepad.registerControl(15, "right");//"d");
    gamepad.registerControl(16, "down");//"s");
    gamepad.registerControl(17, "left");//"a");
    //gamepad.registerControl(3, "up");
    //gamepad.registerControl(2, "right");
    //gamepad.registerControl(1, "down");
    //gamepad.registerControl(0, "left");

    // Force vsync on current context
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    // Lastly, initialize Renderer
    m_renderer = IRendererPtr(new GlfwRenderer(m_window, m_resources));
    if (!m_renderer->init())
        return false;

    return true;
}

void GlfwInstance::pollEvents()
{
    // Try to process events first
    glfwPollEvents();

    // Process gamepads (not handled by glfwPollEvents)
    for (auto& gamepad : m_gamepads)
        gamepad.update(m_scene.get());
}

void GlfwInstance::update(double elapsedTime)
{
    if (isQuit())
        return;

    // Send elapsed time down to game objects
    m_scene->update(elapsedTime);
}

void GlfwInstance::render()
{
    m_renderer->preRender();
    m_scene->render(m_renderer.get());
    m_renderer->postRender();
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
            event.y = floor(y);
            event.w = width;
            event.h = height;
            event.down = (action == GLFW_PRESS);

            instance->m_scene->mouseEvent(event);
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
