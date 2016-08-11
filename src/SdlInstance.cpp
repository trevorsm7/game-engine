#include "SDLInstance.h"
#include "SdlRenderer.h"

#include <cstdio>
#include <OpenGL/gl3.h>

SdlInstance::~SdlInstance()
{
    //if (m_context)
    //    SDL_GL_DeleteContext(m_context);

    if (m_window)
        SDL_DestroyWindow(m_window);

    SDL_Quit();
}

void SdlInstance::run(const char* script)
{
    SdlInstance instance;

    if (!instance.init(script))
        return;

    //int32_t lastTime, currentTime, elapsedTime;
    uint64_t lastTime, currentTime, elapsedTime;
    double period = 1 / float(SDL_GetPerformanceFrequency());

    // The first poll blocks for a relatively long time; get it out of the way before starting game loop
    //lastTime = SDL_GetPerformanceCounter();
    instance.pollEvents();
    //printf("First poll: %f\n", (SDL_GetPerformanceCounter() - lastTime) * period);

    lastTime = SDL_GetPerformanceCounter();
    while (!instance.isQuit())
    {
        // Render first; returns right after vsync
        instance.render();

        // We could improve the input latency by sleeping after vsync to push input/update closer to next vsync
        //SDL_Delay(8);

        // Process event queue
        instance.pollEvents();

        // Compute elapsed time since last update and pass to engine
        // TODO: check for overflow
        currentTime = SDL_GetPerformanceCounter();
        elapsedTime = currentTime - lastTime;
        lastTime = currentTime;
        instance.update(elapsedTime * period);
    }
}

bool SdlInstance::init(const char* script)
{
    //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Testing", "Hello world", nullptr);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        fprintf(stderr, "Failed to init SDL: %s\n", SDL_GetError());
        return false;
    }

    // Text input mode can be enabled by default and can cause significant latency issues; disable it
    if (SDL_IsTextInputActive())
    {
        SDL_StopTextInput();
        printf("Disabling text input mode: %s\n", SDL_IsTextInputActive() ? "failure" : "success");
    }

#if 0
    // Set OpenGL context attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create the window
    m_window = SDL_CreateWindow("Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    //m_window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!m_window)
    {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return false;
    }

    //SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP); // 0, SDL_WINDOW_FULLSCREEN

    // Create the OpenGL context
    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context)
    {
        fprintf(stderr, "Failed to create GL context: %s\n", SDL_GetError());
        return false;
    }

    // Set vertical retrace synchronization
    SDL_GL_SetSwapInterval(1);

    int width, height;
    SDL_GL_GetDrawableSize(m_window, &width, &height);
    printf("Framebuffer size: %d, %d\n", width, height);
#else
    // Create the window
    m_window = SDL_CreateWindow("Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!m_window)
    {
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        return false;
    }
#endif

    m_renderer = IRendererPtr(new SdlRenderer(m_window, m_resources));
    if (!m_renderer->init())
        return false;

    // NOTE: creating window first, then scene can change size if it wants
    m_scene = ScenePtr(new Scene(m_resources));
    m_scene->setQuitCallback([&] {m_bQuit = true;});
    m_scene->setRegisterControlCallback([&](const char* action)->bool
    {
        // TODO: replace with a mechanism for default bindings and loading/saving custom bindings
        //printf("TODO: implement register control callback\n");

        return false;
    });
    if (!m_scene->load(script))
        return false;

    // TODO default window size??
    if (m_scene->isPortraitHint())
        SDL_SetWindowSize(m_window, 480, 640);

    int width, height;
    reinterpret_cast<SdlRenderer*>(m_renderer.get())->getSize(width, height);
    m_scene->resize(width, height);

    //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Testing", "Hello world", m_window);

    return true;
}

void SdlInstance::pollEvents()
{
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
        case SDL_QUIT:
            m_bQuit = true;
            return;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            handleKeyEvent(e.key);
            break;
        case SDL_MOUSEMOTION:
            //if (SDL_GetRelativeMouseMode() == SDL_TRUE)
            //    printf("Mouse motion: %d, %d\n", e.motion.xrel, e.motion.yrel);
            //else
            //    printf("Mouse position: %d, %d\n", e.motion.x, e.motion.y);
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            handleMouseButtonEvent(e.button);
            break;
        case SDL_CONTROLLERDEVICEADDED:
            //SDL_NumJoysticks()
            //SDL_IsGameController(e.cdevice.which)
            {
                SDL_GameController* controller = SDL_GameControllerOpen(e.cdevice.which);
                if (!controller)
                {
                    printf("Failed to acquire controller: %s\n", SDL_GetError());
                    break;
                }
                const char* name = SDL_GameControllerName(controller);
                //const char* name = SDL_GameControllerNameForIndex(e.cdevice.which);
                if (name)
                    printf("%s (%d) added\n", name, e.cdevice.which);
                else
                    printf("Nameless controller %d added\n", e.cdevice.which);
            }
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            {
                SDL_GameController* controller = SDL_GameControllerFromInstanceID(e.cdevice.which);
                if (!controller)
                {
                    printf("Controller instance %d removed; failed: %s\n", e.cdevice.which, SDL_GetError());
                    break;
                }
                const char* name = SDL_GameControllerName(controller);
                if (name)
                    printf("%s (%d) removed\n", name, e.cdevice.which);
                else
                    printf("Controller instance %d removed, no name\n", e.cdevice.which);
                SDL_GameControllerClose(controller);
            }
            break;
        //SDL_CONTROLLERDEVICEREMAPPED
        case SDL_CONTROLLERBUTTONDOWN:
        case SDL_CONTROLLERBUTTONUP:
            handleGamepadButtonEvent(e.cbutton);
            break;
        case SDL_CONTROLLERAXISMOTION:
            handleGamepadAxisEvent(e.caxis);
            break;
        case SDL_WINDOWEVENT:
            //if (SDL_GetWindowFromID(e.window.windowID) != m_window)
            //    break;
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
                int width, height;
                //SDL_GL_GetDrawableSize(m_window, &width, &height);
                reinterpret_cast<SdlRenderer*>(m_renderer.get())->getSize(width, height);
                //printf("Framebuffer size: %d, %d\n", width, height);
                m_scene->resize(width, height);
                break;
            }
            break;
        }
    }
}

void SdlInstance::update(double elapsedTime)
{
    if (isQuit())
        return;

    // Send elapsed time down to game objects
    m_scene->update(elapsedTime);
}

void SdlInstance::render()
{
    m_renderer->preRender();
    m_scene->render(m_renderer.get());
    m_renderer->postRender();
}

void SdlInstance::handleKeyEvent(SDL_KeyboardEvent& e)
{
    // Ignore key repeats
    if (e.repeat)
        return;

    ControlEvent event;
    event.name = nullptr;
    event.down = (e.state == SDL_PRESSED);

    switch (e.keysym.sym)
    {
    case SDLK_UP:
        event.name = "up";
        break;
    case SDLK_LEFT:
        event.name = "left";
        break;
    case SDLK_DOWN:
        event.name = "down";
        break;
    case SDLK_RIGHT:
        event.name = "right";
        break;
    case SDLK_w:
        event.name = "w";
        break;
    case SDLK_a:
        event.name = "a";
        break;
    case SDLK_s:
        event.name = "s";
        break;
    case SDLK_d:
        event.name = "d";
        break;
    case SDLK_SPACE:
        event.name = "action";
        break;
    case SDLK_ESCAPE:
        event.name = "quit";
        break;
    }

    // If we found a mapping, send control to game
    if (event.name && m_scene->controlEvent(event))
        return;

    // Provide default behavior if key not handled by game
    if (e.keysym.sym == SDLK_ESCAPE && e.state == SDL_PRESSED)
        m_bQuit = true;
}

void SdlInstance::handleMouseButtonEvent(SDL_MouseButtonEvent& e)
{
    // TODO: add support for more than one mouse button
    if (e.button != SDL_BUTTON_LEFT)
        return;

    int width, height;
    SDL_GetWindowSize(m_window, &width, &height);

    // Map mouse click to event structure
    MouseEvent event;
    event.x = e.x;
    event.y = e.y;
    event.w = width;
    event.h = height;
    event.down = (e.state == SDL_PRESSED);

    m_scene->mouseEvent(event);
}

void SdlInstance::handleGamepadButtonEvent(SDL_ControllerButtonEvent& e)
{
    ControlEvent event;
    event.name = nullptr;
    event.down = (e.state == SDL_PRESSED);

    const char* name = "?";
    switch (e.button)
    {
    case SDL_CONTROLLER_BUTTON_A:
        name = "A";
        event.name = "action";
        break;
    case SDL_CONTROLLER_BUTTON_B:
        name = "B";
        break;
    case SDL_CONTROLLER_BUTTON_X:
        name = "X";
        break;
    case SDL_CONTROLLER_BUTTON_Y:
        name = "Y";
        break;
    case SDL_CONTROLLER_BUTTON_BACK:
        name = "BACK";
        break;
    case SDL_CONTROLLER_BUTTON_GUIDE:
        name = "GUIDE";
        break;
    case SDL_CONTROLLER_BUTTON_START:
        name = "START";
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSTICK:
        name = "LEFTSTICK";
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSTICK:
        name = "RIGHTSTICK";
        break;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
        name = "LEFTSHOULDER";
        break;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
        name = "RIGHTSHOULDER";
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:
        name = "UP";
        event.name = "up";
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
        name = "DOWN";
        event.name = "down";
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
        name = "LEFT";
        event.name = "left";
        break;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
        name = "RIGHT";
        event.name = "right";
        break;
    }

    //printf("Controller %d: %s %s\n", e.which, name, e.state == SDL_PRESSED ? "pressed" : "released");

    if (!event.name)
        return;

    m_scene->controlEvent(event);
}

void SdlInstance::handleGamepadAxisEvent(SDL_ControllerAxisEvent& e)
{
    const char* name = "?";
    switch (e.axis)
    {
    case SDL_CONTROLLER_AXIS_LEFTX:
        name = "LEFTX";
        break;
    case SDL_CONTROLLER_AXIS_LEFTY:
        name = "LEFTY";
        break;
    case SDL_CONTROLLER_AXIS_RIGHTX:
        name = "RIGHTX";
        break;
    case SDL_CONTROLLER_AXIS_RIGHTY:
        name = "RIGHTY";
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
        name = "TRIGGERLEFT";
        break;
    case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
        name = "TRIGGERRIGHT";
        break;
    }

    //printf("Controller %d: %s %d\n", e.which, name, e.value);
}
