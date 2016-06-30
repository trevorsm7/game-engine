#ifndef __SERIALIZER_H__
#define __SERIALIZER_H__

#include "lua.hpp"
#include <map>
#include <vector>
#include <memory>
#include <utility>
#include <string>

typedef std::pair<std::string, std::string> ImmutableAttrib; // [first] = [second]
typedef std::pair<std::string, const void*> MutableAttrib; // id[.first] = [second] or id[:first]([second])

struct UserdataStore
{
    std::string className; // can use "" for tables
    std::vector<ImmutableAttrib> immAttribs;
    std::vector<MutableAttrib> mutAttribs;
    std::vector<MutableAttrib> cycleAttribs;
    int refCount, refDepth;
    int index;
    bool onStack; // for cycle detection

    UserdataStore(int depth): refCount(1), refDepth(depth), onStack(true) {}
};

typedef std::unique_ptr<UserdataStore> UserdataStorePtr;

class Serializer
{
private:
    std::map<const void*, UserdataStorePtr> m_userdataMap;
    UserdataStore* m_root;

public:
    Serializer(): m_root(nullptr) {}

    UserdataStore* addUserdataStore(const void* userdata, int depth);
    UserdataStore* getUserdataStore(const void* userdata);

    void setAttrib(UserdataStore* store, const char* name, const char* setter, lua_State* L, int index);

    void setAttrib(UserdataStore* store, const char* name, std::string& value)
        {store->immAttribs.emplace_back(name, value);}

    template <class T>
    void setAttribScalar(UserdataStore* store, const char* name, T value)
    {
        std::string strVal = std::to_string(value);
        setAttrib(store, name, strVal);
    }

    template <class T>
    void setAttribArray(UserdataStore* store, const char* name, T val1, T val2)
    {
        std::string val = std::string("{") + std::to_string(val1) + ", " + std::to_string(val2) + "}";
        setAttrib(store, name, val);
    }

    void serializeMutable(UserdataStore* parent, const char* name, const char* setter, lua_State* L, int index);

    void printObject(UserdataStore* ptr, int indent);
    void print();

    static Serializer* checkSerializer(lua_State* L, int index)
    {
        luaL_checktype(L, index, LUA_TLIGHTUSERDATA);
        return reinterpret_cast<Serializer*>(lua_touserdata(L, index));
    }
};

#endif
