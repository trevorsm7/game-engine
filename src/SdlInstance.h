#ifndef __SDLINSTANCE_H__
#define __SDLINSTANCE_H__

#include "IRenderer.h"
#include "ResourceManager.h"
#include "Scene.h"

#include <memory>
#include <SDL2/SDL.h>

class SdlInstance
{
    typedef std::unique_ptr<IRenderer> IRendererPtr;
    typedef std::unique_ptr<Scene> ScenePtr;

    ResourceManager m_resources;
    IRendererPtr m_renderer;
    ScenePtr m_scene;
    SDL_Window* m_window;
    bool m_shouldQuit;
    //SDL_GLContext m_context; // void*

public:
    ~SdlInstance();

    static void run(const char* script);

private:
    SdlInstance(): m_window(nullptr), m_shouldQuit(false) {}//, m_context(nullptr) {}
    bool init(const char* script);
    bool update(double elapsedTime);

    bool handleKeyEvent(SDL_KeyboardEvent& e);
    bool handleMouseButtonEvent(SDL_MouseButtonEvent& e);
    bool handleGamepadButtonEvent(SDL_ControllerButtonEvent& e);
    bool handleGamepadAxisEvent(SDL_ControllerAxisEvent& e);
};

#endif
