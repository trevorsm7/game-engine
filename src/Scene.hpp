#pragma once

#include "Event.hpp"
#include "ResourceManager.hpp"

#include <vector>
#include <memory>
#include <functional>
#include <chrono>

class Canvas;
class IRenderer;
class IAudio;

struct lua_State;
struct lua_Debug;

class Scene
{
    // NOTE: using wrapper instead of func ptr to support state/closures
    typedef std::function<void (void)> QuitCallback;
    typedef std::function<bool (const char*)> RegisterControlCallback;

    ResourceManager& m_resources;
    std::vector<Canvas*> m_canvases;
    std::vector<std::string> m_tempAudioList; // TODO replace with list of AudioSources
    QuitCallback m_quitCallback;
    RegisterControlCallback m_registerControlCallback;
    lua_State* m_L;
    std::chrono::steady_clock::time_point m_watchdog;
    int m_watchdogTotal;
    int m_watchdogCount;
    bool m_isPortraitHint;

public:
    static constexpr const char* const CANVASES = "CANVASES";
    static constexpr const char* const WEAK_REFS = "WEAK_REFS";
    static constexpr const char* const GLOBAL_CHUNK = "GLOBAL_CHUNK";

    Scene(ResourceManager& resources): m_resources(resources), m_L(nullptr), m_isPortraitHint(false) {}
    ~Scene();

    bool load(const char *filename);
    void setQuitCallback(QuitCallback cb) {m_quitCallback = cb;}
    void setRegisterControlCallback(RegisterControlCallback cb) {m_registerControlCallback = cb;}

    bool isPortraitHint() {return m_isPortraitHint;}

    ResourceManager& getResourceManager() {return m_resources;}

    void update(float delta);
    void playAudio(IAudio* audio);
    void render(IRenderer* renderer);
    bool mouseEvent(MouseEvent& event);
    bool controlEvent(ControlEvent& event);
    void resize(int width, int height);

    void setWatchdog(int millis);
    void clearWatchdog();

    static Scene* checkScene(lua_State* L);

private:
    static void acquireCanvas(lua_State* L, Canvas* ptr, int index);
    static void releaseCanvas(lua_State* L, Canvas* ptr);

    static void hook_watchdog(lua_State* L, lua_Debug* ar);

    static int scene_addCanvas(lua_State* L);
    static int scene_loadClosure(lua_State* L);
    static int scene_saveState(lua_State* L);
    static int scene_writeGlobal(lua_State* L);
    static int scene_playSample(lua_State* L); // TODO remove
    static int scene_registerControl(lua_State* L);
    static int scene_setPortraitHint(lua_State* L);
    static int scene_quit(lua_State* L);
};
