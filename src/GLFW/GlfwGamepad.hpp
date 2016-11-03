#pragma once

#include "Event.hpp"
#include "Scene.hpp"

#include <vector>
#include <map>

class GlfwGamepad
{
    int m_gamepad;
    std::vector<float> m_axes;
    std::vector<unsigned char> m_buttons;
    std::map<int, const char*> m_buttonMap;
    bool m_connected;

public:
    GlfwGamepad(int gamepad): m_gamepad(gamepad), m_connected(false) {}

    void registerControl(int button, const char* name) {m_buttonMap[button] = name;}

    // TODO: should we take a Scene*, a GlfwInstance*, or a callback?
    void update(Scene* scene);

    bool isConnected() const {return m_connected;}
};
