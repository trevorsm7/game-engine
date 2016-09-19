#pragma once

#include "lua.hpp"
#include <map>
#include <vector>
#include <memory>
#include <string>
#include <cassert>

class ObjectRef;

class FunctionRef
{
    friend class Serializer;

    struct UpvalueRef {std::string literal; ObjectRef* object; FunctionRef* function;};

private:
    std::string m_name;
    std::string m_code;
    std::vector<UpvalueRef> m_upvalues;
    int m_depth;
    bool m_tempName;

    void setGlobalName(const char* name)
    {
        if (m_tempName)
        {
            m_name = std::string(name);;
            m_tempName = false;
        }
    }

public:
    FunctionRef(int depth): m_depth(depth), m_tempName(true) {}
};

class ObjectRef
{
    friend class Serializer;

    struct InlineRef {std::string table, key; std::string literal; ObjectRef* object;};
    struct SetterRef {std::string setter; ObjectRef* object; FunctionRef* function;};

private:
    std::string m_name;
    std::string m_constructor; // can use "" for tables
    std::vector<InlineRef> m_inlines;
    std::vector<SetterRef> m_setters;
    int m_depth;
    bool m_inlinable;
    bool m_tempName;
    bool m_onStack; // for cycle detection

    void setGlobalName(const char* name)
    {
        if (m_tempName)
        {
            m_name = std::string(name);
            m_inlinable = false;
            m_tempName = false;
        }
    }

public:
    ObjectRef(int depth): m_depth(depth), m_inlinable(true), m_tempName(true), m_onStack(true) {}

    void setConstructor(const char* constructor)
    {
        assert(m_constructor.empty());
        m_constructor = std::string(constructor);
    }

    template <class T>
    void setLiteral(std::string table, std::string key, T value)
    {
        std::string str = std::to_string(value);
        setLiteralRaw(table, key, str);
    }

    template <class T>
    void setArray(std::string table, std::string key, T* value, int length)
    {
        auto str = std::string("{");
        for (int i = 0; i < length; ++i)
        {
            if (i > 0)
                str += ", ";
            str += std::to_string(*(value++));
        }
        str += "}";
        setLiteralRaw(table, key, str);
    }

private:
    void setLiteralRaw(std::string table, std::string key, std::string value);
    void setInlineRef(std::string table, std::string key, ObjectRef* ref);
    void setSetterRef(std::string key, std::string setter, ObjectRef* ref, FunctionRef* func, bool isGlobal = false);
};

template <>
inline void ObjectRef::setLiteral<bool>(std::string table, std::string key, bool value)
{
    setLiteralRaw(table, key, value ? "true" : "false");
}

template <>
inline void ObjectRef::setLiteral<const char*>(std::string table, std::string key, const char* value)
{
    auto str = std::string("\"") + value + "\"";
    setLiteralRaw(table, key, str);
}

template <>
inline void ObjectRef::setLiteral<std::string>(std::string table, std::string key, std::string value)
{
    auto str = std::string("\"") + value + "\"";
    setLiteralRaw(table, key, str);
}

class Serializer
{
    typedef std::unique_ptr<ObjectRef> ObjectRefPtr;
    typedef std::unique_ptr<FunctionRef> FunctionRefPtr;

private:
    ObjectRef m_root;
    std::map<const void*, ObjectRefPtr> m_objects;
    std::map<const void*, FunctionRefPtr> m_functions;
    std::map<const void*, std::string> m_globals;

public:
    Serializer(): m_root(0) {}

    ObjectRef* getObjectRef(const void* ptr);
    FunctionRef* getFunctionRef(const void* ptr);

    void populateGlobals(std::string prefix, lua_State* L, int index);

    void serializeValue(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index);
    void serializeObject(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index);
    void serializeFunction(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index);
    void serializeFromTable(ObjectRef* ref, const char* table, lua_State* L, int index);

    void print();

private:
    ObjectRef* addObjectRef(const void* ptr, int depth);
    FunctionRef* addFunctionRef(const void* ptr, int depth);

    void printSetters(ObjectRef* ref);
    void printInlines(ObjectRef* ref, int indent);
    void printObject(ObjectRef* ref, int indent);
};
