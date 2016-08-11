#pragma once

#include "lua.hpp"
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <cassert>

class ObjectRef
{
    friend class Serializer;

    // TODO map<table> -> vector<key, value> instead?
    struct ImmediateVal {std::string table, key, value;};
    struct ObjectDirect {std::string table, key; const void* ptr;};
    struct ObjectCycle {std::string setter; const void* ptr;};

private:
    std::string m_constructor; // can use "" for tables
    std::vector<ImmediateVal> m_immediates;
    std::vector<ObjectDirect> m_directObjs;
    std::vector<ObjectCycle> m_cyclicObjs;
    int m_count, m_depth;
    int m_index;
    bool m_onStack; // for cycle detection

public:
    ObjectRef(int depth): m_count(1), m_depth(depth), m_onStack(true) {}

    void setConstructor(const char* constructor)
    {
        assert(m_constructor.empty());
        m_constructor = std::string(constructor);
    }

    void setImmediate(std::string table, std::string key, std::string value);

    template <class T>
    void setType(std::string table, std::string key, T value)
    {
        // TODO override for char*, bool, others?
        // TODO use this in serializeValue
        std::string str = std::to_string(value);
        setImmediate(table, key, str);
    }

    template <class T>
    void setArray(std::string table, std::string key, T* value, int length)
    {
        std::string str = std::string("{");
        for (int i = 0; i < length; ++i)
        {
            if (i > 0)
                str += ", ";
            str += std::to_string(*(value++));
        }
        str += std::string("}");
        setImmediate(table, key, str);
    }

private:
    void setObjectDirect(std::string table, std::string key, const void* ptr);
    void setObjectCycle(std::string key, std::string setter, const void* ptr);
};

template <>
void ObjectRef::setType<bool>(std::string table, std::string key, bool value);

template <>
void ObjectRef::setType<const char*>(std::string table, std::string key, const char* value);

typedef std::unique_ptr<ObjectRef> ObjectRefPtr;

class Serializer
{
private:
    std::map<const void*, ObjectRefPtr> m_objectRefs;
    ObjectRef* m_root;

public:
    Serializer(): m_root(nullptr) {}

    ObjectRef* addObjectRef(const void* ptr, int depth);
    ObjectRef* getObjectRef(const void* ptr);

    void serializeValue(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index);
    void serializeObject(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index);
    void serializeFromTable(ObjectRef* ref, const char* table, lua_State* L, int index);

    void print();
    void printObject(ObjectRef* ref, int indent);

    static Serializer* checkSerializer(lua_State* L, int index)
    {
        luaL_checktype(L, index, LUA_TLIGHTUSERDATA);
        return reinterpret_cast<Serializer*>(lua_touserdata(L, index));
    }
};
