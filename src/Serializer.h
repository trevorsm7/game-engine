#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__

#include "lua.hpp"
#include <map>
#include <vector>
//#include <list>
#include <utility>
#include <string>

typedef std::pair<std::string, std::string> ImmutableAttrib;
typedef std::pair<std::string, const void*> MutableAttrib;

struct UserdataStore
{
    std::string className; // use "" for tables so we can reuse?
    std::vector<ImmutableAttrib> immAttribs; // {..., [first] = [second], ...}
    std::vector<MutableAttrib> mutAttribs; // id:[first]([second])? id.[first] = second?

    UserdataStore(std::string name): className(name), immAttribs(), mutAttribs() {}
};

class Serializer
{
private:
    std::vector<UserdataStore> m_userdataList;
    std::map<const void*, size_t> m_userdataMap;

public:
    size_t addUserdataStore(const void* userdata, const char* name);
    //UserdataStore* getUserdataStore(const void* userdata);
    bool hasUserdataStore(const void* userdata);

    // NOTE need to be mindful of what userdata ptr is given virtual hierarchy
    void setAttrib(size_t store, const char* name, lua_State* L, int index);
    void setAttrib(size_t store, const char* name, std::string& value)
        {m_userdataList[store].immAttribs.emplace_back(name, value);}

    template <class T>
    void setAttribScalar(size_t store, const char* name, T value)
    {
        std::string strVal = std::to_string(value);
        setAttrib(store, name, strVal);
    }

    template <class T>
    void setAttribArray(size_t store, const char* name, T val1, T val2)
    {
        std::string val = std::string("{") + std::to_string(val1) + ", " + std::to_string(val2) + "}";
        setAttrib(store, name, val);
    }

    const void* serializeUserdata(lua_State* L, int index);
    const void* serializeTable(lua_State* L, int index);

    void print();

    static Serializer* checkSerializer(lua_State* L, int index)
    {
        luaL_checktype(L, index, LUA_TLIGHTUSERDATA);
        return reinterpret_cast<Serializer*>(lua_touserdata(L, index));
    }
};

#endif
