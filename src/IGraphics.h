#ifndef __IGRAPHICS_H__
#define __IGRAPHICS_H__

#include "TUserdata.h"

#include <cassert>

class Actor;
class IRenderer;

class IGraphics
{
public:
    Actor* m_actor;
protected:
    struct {float r, g, b;} m_color;
    bool m_isVisible;

public:
    IGraphics(): m_actor(nullptr), m_color{1.f, 1.f, 1.f}, m_isVisible(true) {}
    virtual ~IGraphics() {}

    virtual void update(float delta) = 0;
    virtual void render(IRenderer* renderer) = 0;

    // TODO: test click/ray for mouse events
    virtual bool testBounds(float x, float y) const = 0;

    bool isVisible() const {return m_isVisible;}
    void setVisible(bool visible) {m_isVisible = visible;}
    void setColor(float r, float g, float b) {m_color = {r, g, b};}

    static IGraphics* testUserdata(lua_State* L, int index)
    {
        IGraphics* graphics = nullptr;

        // NOTE getmetafield pushes nothing when LUA_TNIL is returned
        if (luaL_getmetafield(L, index, INTERFACE) != LUA_TNIL)
        {
            lua_pop(L, 1);
            graphics = reinterpret_cast<IGraphics*>(lua_touserdata(L, index));
        }

        return graphics;
    }

    static IGraphics* checkUserdata(lua_State* L, int index)
    {
        IGraphics* graphics = testUserdata(L, index);

        if (!graphics)
            luaL_error(L, "expected userdata with interface %s", INTERFACE);

        return graphics;
    }

    // Virtual functions to be implemented by TGraphics template
    virtual bool pushUserdata(lua_State* L) = 0;
    virtual void refAdded(lua_State* L, int index) = 0;
    virtual void refRemoved(lua_State* L) = 0;

protected:
    virtual bool pcall(lua_State* L, const char* method, int in, int out) = 0;

    static constexpr const char* const INTERFACE = "IGraphics";
};

template <class T>
class TGraphics : public IGraphics, public TUserdata<T>
{
public:
    static void initMetatable(lua_State* L);

    bool pushUserdata(lua_State* L) override {return TUserdata<T>::pushUserdata(L);}
    void refAdded(lua_State* L, int index) override {TUserdata<T>::refAdded(L, index);}
    void refRemoved(lua_State* L) override {TUserdata<T>::refRemoved(L);}

protected:
    bool pcall(lua_State* L, const char* method, int in, int out) override
        {return TUserdata<T>::pcall(L, method, in, out);}

    void construct(lua_State* L);
    //void destroy(lua_State* L) {}

    static int script_isVisible(lua_State* L);
    static int script_setVisible(lua_State* L);
    static int script_setColor(lua_State* L);
    static const luaL_Reg METHODS[];
};

template <class T>
const luaL_Reg TGraphics<T>::METHODS[] =
{
    {"isVisible", TGraphics<T>::script_isVisible},
    {"setVisible", TGraphics<T>::script_setVisible},
    {"setColor", TGraphics<T>::script_setColor},
    {nullptr, nullptr}
};

template <class T>
void TGraphics<T>::initMetatable(lua_State* L)
{
    int top = lua_gettop(L);
    TUserdata<T>::initMetatable(L);

    // Push the metatable back on the stack
    luaL_getmetatable(L, T::METATABLE);
    assert(lua_type(L, -1) == LUA_TTABLE);

    // Push the interface name as a key
    lua_pushstring(L, IGraphics::INTERFACE);
    lua_pushboolean(L, true);
    lua_rawset(L, -3);

    // Push interface methods
    // NOTE compiler should trim this if sublcass doesn't define T::METHODS
    if (T::METHODS != TGraphics<T>::METHODS)
    {
        lua_pushliteral(L, "methods");
        lua_rawget(L, -2);
        assert(lua_type(L, -1) == LUA_TTABLE);
        luaL_setfuncs(L, TGraphics<T>::METHODS, 0);
        lua_pop(L, 1);
    }

    // Pop the metatable and method table from the stack
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

template <class T>
void TGraphics<T>::construct(lua_State* L)
{
    lua_pushliteral(L, "visible");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TBOOLEAN);
        setVisible(lua_toboolean(L, -1));
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "color");
    if (lua_rawget(L, 1) != LUA_TNIL)
    {
        // TODO refactor w/ script_setColor
        for (int i = 1; i <= 3; ++i)
            luaL_argcheck(L, (lua_rawgeti(L, -i, i) == LUA_TNUMBER), 1, "color = {r, g, b} must be numbers");
        float r = lua_tonumber(L, -3);
        float g = lua_tonumber(L, -2);
        float b = lua_tonumber(L, -1);
        setColor(r, g, b);
        lua_pop(L, 3);
    }
    lua_pop(L, 1);
}

template <class T>
int TGraphics<T>::script_setVisible(lua_State* L)
{
    // Validate function arguments
    T* self = TUserdata<T>::checkUserdata(L, 1);
    luaL_checktype(L, 2, LUA_TBOOLEAN);

    self->setVisible(lua_toboolean(L, 2));

    // Return self userdata
    lua_pushvalue(L, 1);
    return 1;
}

template <class T>
int TGraphics<T>::script_isVisible(lua_State* L)
{
    // Validate function arguments
    T* self = TUserdata<T>::checkUserdata(L, 1);

    lua_pushboolean(L, self->isVisible());

    return 1;
}

template <class T>
int TGraphics<T>::script_setColor(lua_State* L)
{
    // Validate function arguments
    T* self = TUserdata<T>::checkUserdata(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    for (int i = 1; i <= 3; ++i)
        luaL_argcheck(L, (lua_rawgeti(L, 2, i) == LUA_TNUMBER), 2, "color should have three numbers {r, g, b}");

    float red = lua_tonumber(L, -3);
    float green = lua_tonumber(L, -2);
    float blue = lua_tonumber(L, -1);
    self->setColor(red, green, blue);

    // Return self userdata
    lua_pushvalue(L, 1);
    return 1;
}

#endif
