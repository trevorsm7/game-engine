#pragma once

#include "IUserdata.hpp"

class Actor; // TODO make an Actor component
class IRenderer;

class IPathing : public TUserdata<IPathing>
{
public:
    Actor* m_actor;

protected:
    IPathing(): m_actor(nullptr) {}

public:
    virtual ~IPathing() {}

    virtual void update(float delta) = 0;
    virtual void render(IRenderer* renderer) = 0; // for debugging

    virtual bool findPath(int x1, int y1, int x2, int y2, int& xOut, int& yOut) = 0;

private:
    friend class TUserdata<IPathing>;
    //void construct(lua_State* L);
    //void destroy(lua_State* L) {}
    //void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_findPath(lua_State* L);
    static int script_clearPath(lua_State* L);

    static constexpr const char* const CLASS_NAME = "IPathing";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"findPath", script_findPath},
        {"clearPath", script_clearPath},
        {nullptr, nullptr}
    };
};
