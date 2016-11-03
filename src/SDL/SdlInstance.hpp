#pragma once

#include "IAudio.hpp"
#include "IRenderer.hpp"
#include "ResourceManager.hpp"
#include "Scene.hpp"

#include <memory>
#include <SDL2/SDL.h>

class SdlInstance
{
    typedef std::unique_ptr<IAudio> IAudioPtr;
    typedef std::unique_ptr<IRenderer> IRendererPtr;
    typedef std::unique_ptr<Scene> ScenePtr;

    ResourceManager m_resources;
    IRendererPtr m_renderer;
    IAudioPtr m_audio;
    ScenePtr m_scene;
    bool m_bQuit;

public:
    ~SdlInstance();

    static void run(const char* script);

private:
    SdlInstance(): m_bQuit(false) {}

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
