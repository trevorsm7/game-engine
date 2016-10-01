#pragma once

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <initializer_list>

struct lua_State;

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

    void setGlobalName(const std::string& name)
    {
        // Only set name the first time this is called
        if (m_tempName)
        {
            m_name = name;
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

    void setGlobalName(const std::string& name)
    {
        // Only set name the first time this is called
        if (m_tempName)
        {
            m_name = name;
            m_inlinable = false;
            m_tempName = false;
        }
    }

public:
    ObjectRef(int depth, bool inlinable): m_depth(depth), m_inlinable(inlinable), m_tempName(true), m_onStack(true) {}

    void setConstructor(const std::string& constructor)
    {
        m_constructor = constructor;
    }

    void setString(const std::string& table, const std::string& key, const std::string& value)
    {
        setLiteral(table, key, std::string("\"") + value + "\"");
    }

    void setBoolean(const std::string& table, const std::string& key, bool value)
    {
        setLiteral(table, key, value ? "true" : "false");
    }

    template <class T>
    void setNumber(const std::string& table, const std::string& key, T value)
    {
        setLiteral(table, key, std::to_string(value));
    }

    template <class T>
    void setArray(const std::string& table, const std::string& key, T* value, int length)
    {
        auto str = std::string("{");
        for (int i = 0; i < length; ++i)
        {
            if (i > 0)
                str += ", ";
            str += std::to_string(*(value++));
        }
        str += "}";
        setLiteral(table, key, str);
    }

private:
    void setLiteral(const std::string& table, const std::string& key, const std::string& value);
    void setInlineRef(const std::string& table, const std::string& key, ObjectRef* ref);
    void setSetterRef(const std::string& key, const std::string& setter, ObjectRef* ref, FunctionRef* func);
};

/*template <>
inline void ObjectRef::setNumber<float>(std::string table, std::string key, float value)
{
    setLiteral(table, key, std::string("string.unpack(\"<f\",") + ???? + ")");
}*/

class Serializer
{
    typedef std::unique_ptr<ObjectRef> ObjectRefPtr;
    typedef std::unique_ptr<FunctionRef> FunctionRefPtr;

    struct SetterArg {std::string literal; ObjectRef* object; FunctionRef* function;};
    struct SetterRef {std::string setter; std::vector<SetterArg> args;};

private:
    ObjectRef m_root;
    std::map<const void*, ObjectRefPtr> m_objects;
    std::map<const void*, FunctionRefPtr> m_functions;
    std::map<const void*, std::string> m_globals;
    std::vector<SetterRef> m_setters;

public:
    Serializer(): m_root(0, false) {}

    ObjectRef* getObjectRef(const void* ptr)
    {
        auto it = m_objects.find(ptr);
        if (it == m_objects.end())
            return nullptr;

        return it->second.get();
    }

    FunctionRef* getFunctionRef(const void* ptr)
    {
        auto it = m_functions.find(ptr);
        if (it == m_functions.end())
            return nullptr;

        return it->second.get();
    }

    void populateGlobals(const std::string& prefix, lua_State* L, int index);
    std::string* getGlobalName(lua_State* L, int index);

    void serializeSubtable(ObjectRef* parent, const std::string& table, lua_State* L, int index);
    void serializeMember(ObjectRef* parent, const std::string& table, const std::string& key, const std::string& setter, lua_State* L, int index);
    void serializeUpvalue(FunctionRef* parent, lua_State* L, int index);

    void serializeSetter(const std::string& setter, lua_State* L, std::initializer_list<int> list);

    void print();

private:
    ObjectRef* serializeObject(int depth, bool inlinable, lua_State* L, int index);
    FunctionRef* serializeFunction(int depth, lua_State* L, int index);

    void printSetters(ObjectRef* ref);
    void printInlines(ObjectRef* ref, int indent);
    void printObject(ObjectRef* ref, int indent);
};
