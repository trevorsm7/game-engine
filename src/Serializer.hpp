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
    std::string m_name;
    std::string m_constructor; // can use "" for tables
    std::vector<ImmediateVal> m_immediates;
    std::vector<ObjectDirect> m_directObjs;
    std::vector<ObjectCycle> m_cyclicObjs;
    int m_depth;
    bool m_inlinable;
    bool m_tempName;
    bool m_onStack; // for cycle detection

public:
    ObjectRef(int depth): m_depth(depth), m_inlinable(true), m_tempName(true), m_onStack(true) {}

    bool hasImmediates() const {return m_immediates.size() > 0 || m_directObjs.size() > 0;}

    void setGlobalName(const char* name)
    {
        m_name = std::string(name);
        m_inlinable = false;
        m_tempName = false;
    }

    void setConstructor(const char* constructor)
    {
        assert(m_constructor.empty());
        m_constructor = std::string(constructor);
    }

    void setImmediate(std::string table, std::string key, std::string value);

    template <class T>
    void setType(std::string table, std::string key, T value)
    {
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
    void setObjectCycle(std::string key, std::string setter, const void* ptr, bool isRoot = false);
};

template <>
void ObjectRef::setType<bool>(std::string table, std::string key, bool value);

template <>
void ObjectRef::setType<const char*>(std::string table, std::string key, const char* value);

typedef std::unique_ptr<ObjectRef> ObjectRefPtr;

class Serializer
{
private:
    ObjectRef m_root;
    std::map<const void*, ObjectRefPtr> m_objectRefs;
    std::map<const void*, std::string> m_globals;

public:
    Serializer(): m_root(0) {}

    ObjectRef* getObjectRef(const void* ptr);

    void populateGlobals(std::string prefix, lua_State* L, int index);

    void serializeValue(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index);
    void serializeObject(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index, bool forceCycle = false);
    void serializeFromTable(ObjectRef* ref, const char* table, lua_State* L, int index);

    void print();

private:
    ObjectRef* addObjectRef(const void* ptr, int depth);

    void printCycles(ObjectRef* parent);
    void printImmediates(ObjectRef* ref, int indent);
    void printObject(ObjectRef* ref, int indent);
};
