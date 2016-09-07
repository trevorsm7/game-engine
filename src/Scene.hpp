#pragma once

#include "Event.hpp"
#include "ResourceManager.hpp"

#include <vector>
#include <memory>
#include <functional>
#include "lua.hpp"

class Canvas;
class IRenderer;

class Scene
{
    // NOTE: using wrapper instead of func ptr to support state/closures
    typedef std::function<void (void)> QuitCallback;
    typedef std::function<bool (const char*)> RegisterControlCallback;

    ResourceManager& m_resources;
    std::vector<Canvas*> m_canvases;
    QuitCallback m_quitCallback;
    RegisterControlCallback m_registerControlCallback;
    lua_State *m_L;
    bool m_isPortraitHint;

public:
    Scene(ResourceManager& resources): m_resources(resources), m_L(nullptr), m_isPortraitHint(false) {}
    ~Scene() {if (m_L) lua_close(m_L);} // TODO remove Canvas references; will be deleted anyway when closing Lua state, but would be nice to do?

    bool load(const char *filename);
    void setQuitCallback(QuitCallback cb) {m_quitCallback = cb;}
    void setRegisterControlCallback(RegisterControlCallback cb) {m_registerControlCallback = cb;}

    bool isPortraitHint() {return m_isPortraitHint;}

    ResourceManager& getResourceManager() {return m_resources;}

    void update(float delta);
    void render(IRenderer* renderer);
    bool mouseEvent(MouseEvent& event);
    bool controlEvent(ControlEvent& event);
    void resize(int width, int height);

//private:
    static Scene* checkScene(lua_State* L);
    static void addCanvas(lua_State* L, int index);

    static int scene_serialize(lua_State* L);
    static int scene_writeGlobal(lua_State* L);
    static int scene_registerControl(lua_State* L);
    static int scene_setPortraitHint(lua_State* L);
    static int scene_quit(lua_State* L);
};
