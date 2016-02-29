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

    uint32_t lastTime = SDL_GetTicks();
    uint32_t elapsedTime = 0;
    while (instance.update(elapsedTime * 0.001))
    {
        // Compute time since last update
        uint32_t currentTime = SDL_GetTicks();
        elapsedTime = currentTime - lastTime;
        lastTime = currentTime;
    }
}

bool SdlInstance::init(const char* script)
{
    //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Testing", "Hello world", nullptr);

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Failed to init SDL video: %s\n", SDL_GetError());
        return false;
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

    m_renderer = IRendererPtr(new SdlRenderer(m_window, m_resources));
    if (!m_renderer->init())
        return false;

    // NOTE: creating window first, then scene can change size if it wants
    m_scene = ScenePtr(new Scene());
    m_scene->setQuitCallback([&] {m_shouldQuit = true;});
    m_scene->setRegisterControlCallback([&](const char* action)->bool
    {
        // TODO: replace with a mechanism for default bindings and loading/saving custom bindings
        printf("TODO: implement register control callback\n");

        return false;
    });
    if (!m_scene->load(script))
        return false;

    int width, height;
    reinterpret_cast<SdlRenderer*>(m_renderer.get())->getSize(width, height);
    m_scene->resize(width, height);
#endif

    //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Testing", "Hello world", m_window);

    return true;
}

bool SdlInstance::update(double elapsedTime)
{
    // Poll for events
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
        case SDL_QUIT:
            return false;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            if (!handleKeyEvent(e.key))
                if (e.key.keysym.sym == SDLK_ESCAPE && e.key.state == SDL_PRESSED)
                    return false;
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

        if (m_shouldQuit)
            return false;
    }

    // Send elapsed time down to game objects
    m_scene->update(elapsedTime);

    // Render last after input and updates
#if 0
    // Render display
    glClearColor(0.3, 0.3, 0.3, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(m_window);
#else
    m_renderer->preRender();
    m_scene->render(m_renderer.get());
    m_renderer->postRender();
#endif

    return !m_shouldQuit;
}

bool SdlInstance::handleKeyEvent(SDL_KeyboardEvent& e)
{
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

    return m_scene->controlEvent(event);
}

bool SdlInstance::handleMouseButtonEvent(SDL_MouseButtonEvent& e)
{
    if (!m_scene)
        return false;

    if (e.button != SDL_BUTTON_LEFT)
        return false;

    int width, height;
    SDL_GetWindowSize(m_window, &width, &height);

    MouseEvent event;
    event.x = e.x;
    event.y = height - e.y - 1;
    event.w = width;
    event.h = height;
    event.down = (e.state == SDL_PRESSED);

    // TODO: should mouse event return boolean?
    m_scene->mouseEvent(event);
    return true;
}
