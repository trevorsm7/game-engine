#ifndef __COMPONENT_H__
#define __COMPONENT_H__

#include <memory>
#include "lua.hpp"

class Component
{
public:
    virtual ~Component() {}

    virtual void update(lua_State *state, float delta) = 0;
};

typedef std::unique_ptr<Component> ComponentPtr;

#endif
