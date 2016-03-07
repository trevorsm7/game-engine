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
    bool m_bQuit;
    //SDL_GLContext m_context; // void*

public:
    ~SdlInstance();

    static void run(const char* script);

private:
    SdlInstance(): m_window(nullptr), m_bQuit(false) {}//, m_context(nullptr) {}

    bool init(const char* script);
    void pollEvents();
    void update(double elapsedTime);
    void render();

    bool isQuit() const {return m_bQuit;}

    void handleKeyEvent(SDL_KeyboardEvent& e);
    void handleMouseButtonEvent(SDL_MouseButtonEvent& e);
    void handleGamepadButtonEvent(SDL_ControllerButtonEvent& e);
    void handleGamepadAxisEvent(SDL_ControllerAxisEvent& e);
};

#endif
