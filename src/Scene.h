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
    std::vector<Canvas*> m_canvases;
    lua_State *m_state;
    std::function<void (void)> m_quitCallback;

public:
    Scene(): m_state(nullptr) {}
    ~Scene() {if (m_state) lua_close(m_state);}

    void load(const char *filename);
    void setQuitCallback(std::function<void (void)> quitCallback) {m_quitCallback = quitCallback;}

    void update(float delta);
    void render(IRenderer* renderer);
    void mouseEvent(MouseEvent& event);
    //bool keyEvent(std::string& key, bool down);
    bool keyEvent(KeyEvent& event);
    void resize(int width, int height);

//private:
    void addCanvas(Canvas *canvas);

    static int scene_registerKey(lua_State *L);
    static int scene_quit(lua_State *L);
};

#endif
