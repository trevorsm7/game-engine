#pragma once

struct MouseEvent
{
    int x, y;
    int w, h;
    bool down;
};

struct ControlEvent
{
    const char* name;
    bool down;
};
