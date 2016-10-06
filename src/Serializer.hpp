#pragma once

#include <map>
#include <list>
#include <vector>
#include <memory>
#include <string>
#include <limits>
#include <initializer_list>

struct lua_State;

class Serializer;

class ILuaRef
{
public:
    virtual std::string getAsKey() const = 0;
    virtual std::string getAsSetter() const = 0;
    virtual std::string getAsValue() const = 0;

    virtual int getDepth() const = 0;
    virtual bool isOnStack() const = 0;

    // TODO rework this into getAsValue, returning a potentially large string
    virtual void print(Serializer* serializer, int indent, bool isInline) const = 0;
};

class LiteralRef : public ILuaRef
{
    friend class Serializer;

private:
    std::string m_literal;

public:
    LiteralRef(const std::string& literal): m_literal(literal) {}

    std::string getAsKey() const override
    {
        if (m_literal.empty() || m_literal.front() == ':') // TODO the latter would be an error?
            return m_literal;

        if (m_literal.front() == '.')
            return m_literal.substr(1);

        if (m_literal.front() == '\"' && m_literal.back() == '\"')
            return m_literal.substr(1, m_literal.size() - 2);

        return std::string("[") + m_literal + "]";
    }

    std::string getAsSetter() const override
    {
        if (m_literal.empty())
            return m_literal;

        if (m_literal[0] == '.' || m_literal[0] == ':')
            return m_literal;

        if (m_literal.front() == '\"' && m_literal.back() == '\"')
            return std::string(".") + m_literal.substr(1, m_literal.size() - 2);

        return std::string("[") + m_literal + "]";
    }

    std::string getAsValue() const override {return m_literal;}

    int getDepth() const override {return std::numeric_limits<int>::max();}
    bool isOnStack() const override {return false;}

    void print(Serializer* serializer, int indent, bool isInline) const override
    {
        printf("%s", m_literal.c_str());
    }
};

class FunctionRef : public ILuaRef
{
    friend class Serializer;

private:
    std::string m_name;
    std::string m_code;
    std::vector<ILuaRef*> m_upvalues;
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

    std::string getAsKey() const override {return std::string("[") + m_name + "]";}
    std::string getAsSetter() const override {return std::string("[") + m_name + "]";}
    std::string getAsValue() const override {return m_name;} // TODO replace the direct refs to m_name with this?

    int getDepth() const override {return m_depth;}
    bool isOnStack() const override {return false;}

    void print(Serializer* serializer, int indent, bool isInline) const override
    {
        if (isInline)
        {
            printf("%s", m_name.c_str());
            return;
        }

        printf("loadClosure(\"%s\"", m_code.c_str());
        for (auto& upvalue : m_upvalues)
            printf(", %s", upvalue->getAsValue().c_str());
        printf(")");
    }
};

class ObjectRef : public ILuaRef
{
    friend class Serializer;

    struct InlineRef {std::string table; ILuaRef* key; ILuaRef* value;};
    struct SetterRef {ILuaRef* setter; ILuaRef* value;};

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

    void setConstructor(const std::string& constructor) {m_constructor = constructor;}

    std::string getAsKey() const override {return std::string("[") + m_name + "]";}
    std::string getAsSetter() const override {return std::string("[") + m_name + "]";}
    std::string getAsValue() const override {return m_name;}

    int getDepth() const override {return m_depth;}
    bool isOnStack() const override {return m_onStack;}

    void print(Serializer* serializer, int indent, bool isInline) const override;

private:
    void setInlineRef(const std::string& table, ILuaRef* key, ILuaRef* value);
    void setSetterRef(ILuaRef* setter, ILuaRef* value);
};

class Serializer
{
    typedef std::unique_ptr<ObjectRef> ObjectRefPtr;
    typedef std::unique_ptr<FunctionRef> FunctionRefPtr;
    typedef std::unique_ptr<LiteralRef> LiteralRefPtr;

    struct SetterRef {std::string setter; std::vector<ILuaRef*> args;};

private:
    ObjectRef m_root;
    std::map<const void*, ObjectRefPtr> m_objects;
    std::map<const void*, FunctionRefPtr> m_functions;
    std::map<const void*, std::string> m_globals;
    std::vector<LiteralRefPtr> m_literals;
    std::vector<SetterRef> m_setters;

    template <class T>
    void sortRefsByDepth(const std::map<const void*, std::unique_ptr<T>>& in, std::list<T*>& out)
    {
        for (auto& pair : in)
        {
            T* ref = pair.second.get();

            // Find first index with lower ref depth
            int depth = ref->m_depth;
            auto it = std::find_if(out.begin(), out.end(),
                [depth](T* ptr) {return ptr->m_depth < depth;});

            // Insert before index with lower ref depth
            out.insert(it, ref);
        }
    }

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

    void setString(ObjectRef* ref, const std::string& table, const std::string& key, const std::string& value)
    {
        setLiteral(ref, table, key, std::string("\"") + value + "\"");
    }

    void setBoolean(ObjectRef* ref, const std::string& table, const std::string& key, bool value)
    {
        setLiteral(ref, table, key, value ? "true" : "false");
    }

    template <class T>
    void setNumber(ObjectRef* ref, const std::string& table, const std::string& key, T value)
    {
        setLiteral(ref, table, key, std::to_string(value));
        //setLiteral(table, key, std::string("string.unpack(\"<f\",") + ???? + ")");
    }

    template <class T>
    void setArray(ObjectRef* ref, const std::string& table, const std::string& key, T* value, int length)
    {
        auto str = std::string("{");
        for (int i = 0; i < length; ++i)
        {
            if (i > 0)
                str += ", ";
            str += std::to_string(*(value++));
        }
        str += "}";
        setLiteral(ref, table, key, str);
    }

    void populateGlobals(const std::string& prefix, lua_State* L, int index);
    std::string* getGlobalName(lua_State* L, int index);

    // TODO rename this along the lines of set* similar to setString, setBoolean, etc above
    void serializeSubtable(ObjectRef* parent, const std::string& table, lua_State* L, int index);
    void serializeMember(ObjectRef* parent, const std::string& table, const std::string& key, const std::string& setter, lua_State* L, int index);
    void serializeMember(ObjectRef* parent, const std::string& table, ILuaRef* key, lua_State* L, int index);
    void serializeSetter(const std::string& setter, lua_State* L, std::initializer_list<int> list);

    void print();

private:
    void serializeUpvalue(FunctionRef* parent, lua_State* L, int index);

    void setLiteral(ObjectRef* ref, const std::string& table, const std::string& key, const std::string& value)
    {
        LiteralRef* keyRef = serializeLiteral(std::string(".") + key);
        LiteralRef* valueRef = serializeLiteral(value);
        ref->setInlineRef(table, keyRef, valueRef);
    }

    void setSetter(ObjectRef* ref, const std::string& key, const std::string& setter, ILuaRef* value)
    {
        LiteralRef* setterRef;
        if (!setter.empty())
            setterRef = serializeLiteral(std::string(":") + setter);
        else
            setterRef = serializeLiteral(std::string(".") + key);
        ref->setSetterRef(setterRef, value);
    }

    LiteralRef* serializeLiteral(const std::string& literal)
    {
        LiteralRef* ref = new LiteralRef(literal);
        m_literals.emplace_back(ref);
        return ref;
    }

    ILuaRef* serializeKey(int depth, lua_State* L, int index);
    ObjectRef* serializeObject(int depth, bool inlinable, lua_State* L, int index);
    FunctionRef* serializeFunction(int depth, lua_State* L, int index);

    void printSetters(const ObjectRef* ref);
public: // TODO making this public for now so ObjectRef can access... should move it there
    void printInlines(const ObjectRef* ref, int indent);
};
