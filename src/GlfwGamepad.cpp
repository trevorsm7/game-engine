#include "GlfwGamepad.h"

#include <cstdio>

#define GLFW_INCLUDE_GLCOREARB
#include "GLFW/glfw3.h"

void GlfwGamepad::update(Scene* scene)
{
    // Check if the gamepad has just been connected
    int present = glfwJoystickPresent(m_gamepad);
    if (present != m_connected)
    {
        if (present)
        {
            const char* name = glfwGetJoystickName(m_gamepad);
            printf("Gamepad %d connected: %s\n", m_gamepad + 1, name);
        }
        else
        {
            printf("Gamepad %d disconnected\n", m_gamepad + 1);
            m_axes.clear();
            m_buttons.clear();
        }
        m_connected = present;
    }

    if (m_connected)
    {
        // Get current state of axes and buttons
        int nAxes, nButtons;
        const float* axes = glfwGetJoystickAxes(m_gamepad, &nAxes);
        const unsigned char* buttons = glfwGetJoystickButtons(m_gamepad, &nButtons);

        // Resize saved axes state if size has changed
        // TODO: do we need/want to save analog state?
        if (nAxes != m_axes.size())
        {
            m_axes.resize(nAxes);
            printf("Gamepad %d resizing axes: %d\n", m_gamepad + 1, nAxes);
        }

        // Resize saved button state if size has changed
        if (nButtons != m_buttons.size())
        {
            m_buttons.resize(nButtons);
            printf("Gamepad %d resizing buttons: %d\n", m_gamepad + 1, nButtons);
        }

        // Loop over each control mapped to this gamepad
        for (auto& button : m_buttonMap)
        {
            // If the state of the button has changed, generate an event
            if (buttons[button.first] != m_buttons[button.first])
            {
                ControlEvent event;
                event.name = button.second;
                event.down = buttons[button.first];
                //printf("Gamepad %d - %s\n", m_gamepad + 1, event.down ? "pressed" : "released");
                scene->controlEvent(event);
            }
        }

        // Copy current state to saved state
        m_buttons.assign(buttons, buttons + nButtons);
        m_axes.assign(axes, axes + nAxes);
    }
}
