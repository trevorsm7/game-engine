#ifndef __SCENE_H__
#define __SCENE_H__

#include "IRenderer.h"
#include "Canvas.h"
#include "Event.h"

#include <vector>
#include <memory>
#include <functional>
#include "lua.hpp"

class Scene
{
    // NOTE: using wrapper instead of func ptr to support state/closures
    typedef std::function<void (void)> QuitCallback;
    typedef std::function<bool (const char*)> RegisterControlCallback;

    std::vector<Canvas*> m_canvases;
    lua_State *m_state;
    QuitCallback m_quitCallback;
    RegisterControlCallback m_registerControlCallback;

public:
    Scene(): m_state(nullptr) {}
    ~Scene() {if (m_state) lua_close(m_state);}

    bool load(const char *filename);
    void setQuitCallback(QuitCallback cb) {m_quitCallback = cb;}
    void setRegisterControlCallback(RegisterControlCallback cb) {m_registerControlCallback = cb;}

    void update(float delta);
    void render(IRenderer* renderer);
    void mouseEvent(MouseEvent& event);
    bool controlEvent(ControlEvent& event);
    void resize(int width, int height);

//private:
    void addCanvas(Canvas *canvas);

    static int scene_registerControl(lua_State *L);
    static int scene_quit(lua_State *L);
};

#endif
