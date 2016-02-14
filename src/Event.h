#ifndef __EVENT_H__
#define __EVENT_H__

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

#endif
