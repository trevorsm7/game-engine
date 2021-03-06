#include "SDLInstance.hpp"
#include "SdlRenderer.hpp"
#include "SdlAudio.hpp"

#include <cstdio>
#include "SDL_opengl.h"

SdlInstance::~SdlInstance()
{
    SDL_Quit();
}

void SdlInstance::run(const char* script)
{
    SdlInstance instance;

    if (!instance.init(script))
        return;

    //int32_t lastTime, currentTime, elapsedTime;
    uint64_t lastTime, currentTime, elapsedTime;
    float period = 1.f / float(SDL_GetPerformanceFrequency());

    // The first poll blocks for a relatively long time; get it out of the way before starting game loop
    //lastTime = SDL_GetPerformanceCounter();
    instance.pollEvents();
    //fprintf(stderr, "First poll: %f\n", (SDL_GetPerformanceCounter() - lastTime) * period);

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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        fprintf(stderr, "Failed to init SDL: %s\n", SDL_GetError());
        return false;
    }

    // Make sure text input mode is off when we launch
    //fprintf(stderr, "Text Input mode is %s\n", SDL_IsTextInputActive() ? "on" : "off");
    SDL_StopTextInput();

    m_scene = ScenePtr(new Scene(m_resources));
    m_scene->setQuitCallback([&] {m_bQuit = true;});
    m_scene->setRegisterControlCallback([&](const char* /*action*/)->bool
    {
        // TODO: replace with a mechanism for default bindings and loading/saving custom bindings
        //fprintf(stderr, "TODO: implement register control callback\n");
        return false;
    });

    if (!m_scene->load(script))
        return false;

    // TODO define default window size somewhere
    int width, height;
    if (m_scene->isPortraitHint())
    {
        width = 600;
        height = 800;
    }
    else
    {
        width = 800;
        height = 600;
    }

    SdlRenderer* renderer = new SdlRenderer(m_resources);
    m_renderer = IRendererPtr(renderer);
    if (!renderer->init(width, height))
    {
        m_renderer = nullptr;
        return false;
    }

    m_scene->resize(width, height);

    SdlAudio* audio = new SdlAudio(m_resources);
    m_audio = IAudioPtr(audio);
    if (!audio->init())
    {
        m_audio = nullptr;
        return false;
    }

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
            //    fprintf(stderr, "Mouse motion: %d, %d\n", e.motion.xrel, e.motion.yrel);
            //else
            //    fprintf(stderr, "Mouse position: %d, %d\n", e.motion.x, e.motion.y);
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
                    fprintf(stderr, "Failed to acquire controller: %s\n", SDL_GetError());
                    break;
                }
                const char* name = SDL_GameControllerName(controller);
                //const char* name = SDL_GameControllerNameForIndex(e.cdevice.which);
                if (name)
                    fprintf(stderr, "%s (%d) added\n", name, e.cdevice.which);
                else
                    fprintf(stderr, "Nameless controller %d added\n", e.cdevice.which);
            }
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            {
                SDL_GameController* controller = SDL_GameControllerFromInstanceID(e.cdevice.which);
                if (!controller)
                {
                    fprintf(stderr, "Controller instance %d removed; failed: %s\n", e.cdevice.which, SDL_GetError());
                    break;
                }
                const char* name = SDL_GameControllerName(controller);
                if (name)
                    fprintf(stderr, "%s (%d) removed\n", name, e.cdevice.which);
                else
                    fprintf(stderr, "Controller instance %d removed, no name\n", e.cdevice.which);
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
                //int width, height;
                //SDL_GL_GetDrawableSize(m_window, &width, &height);
                //SDL_GetRendererOutputSize(m_renderer, &width, &height);
                //fprintf(stderr, "Framebuffer size: %dx%d\n", width, height);
                //fprintf(stderr, "Window size: %dx%d\n", e.window.data1, e.window.data2);
                // NOTE just using logical window size instead of framebuffer pixel size
                m_scene->resize(e.window.data1, e.window.data2);
                break;
            }
            break;
        }
    }
}

void SdlInstance::update(float elapsedTime)
{
    if (isQuit())
        return;

    // Send elapsed time down to game objects
    m_scene->update(elapsedTime);
}

void SdlInstance::render()
{
    m_scene->playAudio(m_audio.get());

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
    SDL_Window* window = SDL_GetWindowFromID(e.windowID);
    SDL_GetWindowSize(window, &width, &height);

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

    //fprintf(stderr, "Controller %d: %s %s\n", e.which, name, e.state == SDL_PRESSED ? "pressed" : "released");

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

    //fprintf(stderr, "Controller %d: %s %d\n", e.which, name, e.value);
}
