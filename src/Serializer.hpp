#pragma once

#include <unordered_map>
#include <list>
#include <vector>
#include <memory>
#include <string>
#include <limits>
#include <initializer_list>
#include <cassert>

struct lua_State;

class Serializer;

class ILuaRef
{
public:
    virtual void setGlobalName(const std::string& name) = 0;

    virtual std::string getAsKey() const = 0;
    virtual std::string getAsSetter() const = 0;
    virtual std::string getAsValue() const = 0;

    virtual int getDepth() const = 0;
    virtual bool isSetterOnly() const = 0;

    // TODO rework this into getAsValue, returning a potentially large string
    virtual void print(int indent, bool isInline) const = 0;
};

class KeyRef : public ILuaRef
{
    friend class Serializer;

private:
    std::string m_key, m_setter;

    KeyRef(const std::string& key, const std::string& setter):
        m_key(key), m_setter(setter) {}

public:
    void setGlobalName(const std::string& name) override {};

    std::string getAsKey() const override {assert(!m_key.empty()); return m_key;}

    std::string getAsSetter() const override
    {
        if (m_setter.empty())
        {
            assert(!m_key.empty());
            return std::string(".") + m_key;
        }

        return std::string(":") + m_setter;
    }

    std::string getAsValue() const override {assert(false); return std::string("nil");}

    int getDepth() const override {return std::numeric_limits<int>::max();}
    bool isSetterOnly() const override {return m_key.empty();}

    void print(int indent, bool isInline) const override {assert(false);}
};

class LiteralRef : public ILuaRef
{
    friend class Serializer;

private:
    std::string m_literal;

public:
    LiteralRef(const std::string& literal): m_literal(literal) {}

    void setGlobalName(const std::string& name) override {};

    std::string getAsKey() const override
    {
        assert(!m_literal.empty());

        if (m_literal.front() == '\"' && m_literal.back() == '\"')
            return m_literal.substr(1, m_literal.size() - 2);

        return std::string("[") + m_literal + "]";
    }

    std::string getAsSetter() const override
    {
        assert(!m_literal.empty());

        if (m_literal.front() == '\"' && m_literal.back() == '\"')
            return std::string(".") + m_literal.substr(1, m_literal.size() - 2);

        return std::string("[") + m_literal + "]";
    }

    std::string getAsValue() const override {return m_literal;}

    int getDepth() const override {return std::numeric_limits<int>::max();}
    bool isSetterOnly() const override {return false;}

    void print(int indent, bool isInline) const override
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

public:
    FunctionRef(int depth): m_depth(depth), m_tempName(true) {}

    void setGlobalName(const std::string& name) override
    {
        // Only set name the first time this is called
        if (m_tempName)
        {
            m_name = name;
            m_tempName = false;
        }
    }

    std::string getAsKey() const override {return std::string("[") + m_name + "]";}
    std::string getAsSetter() const override {return std::string("[") + m_name + "]";}
    std::string getAsValue() const override {return m_name;} // TODO replace the direct refs to m_name with this?

    int getDepth() const override {return m_depth;}
    bool isSetterOnly() const override {return true;}

    void print(int indent, bool isInline) const override
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

public:
    void setConstructor(const std::string& constructor) {m_constructor = constructor;}

private:
    std::string m_name;
    std::string m_constructor; // can use "" for tables
    std::vector<InlineRef> m_inlines;
    std::vector<SetterRef> m_setters;
    int m_depth;
    bool m_inlinable;
    bool m_tempName;
    bool m_onStack; // for cycle detection

public:
    ObjectRef(int depth, bool inlinable): m_depth(depth), m_inlinable(inlinable), m_tempName(true), m_onStack(true) {}

    void setGlobalName(const std::string& name) override
    {
        // Only set name the first time this is called
        if (m_tempName)
        {
            m_name = name;
            m_inlinable = false;
            m_tempName = false;
        }
    }

    std::string getAsKey() const override {return std::string("[") + m_name + "]";}
    std::string getAsSetter() const override {return std::string("[") + m_name + "]";}
    std::string getAsValue() const override {return m_name;}

    int getDepth() const override {return m_depth;}
    bool isSetterOnly() const override {return m_onStack;}

    void print(int indent, bool isInline) const override;

private:
    void printInlines(int indent, bool isRoot) const;
    void printSetters() const;

    void setInlineRef(const std::string& table, ILuaRef* key, ILuaRef* value);
    void setSetterRef(ILuaRef* setter, ILuaRef* value);
};

class Serializer
{
    typedef std::unique_ptr<ObjectRef> ObjectRefPtr;
    typedef std::unique_ptr<FunctionRef> FunctionRefPtr;
    typedef std::unique_ptr<LiteralRef> LiteralRefPtr;
    typedef std::unique_ptr<KeyRef> KeyRefPtr;

    struct SetterRef {std::string setter; std::vector<ILuaRef*> args;};

private:
    ObjectRef m_root;
    LiteralRef m_true, m_false, m_nil;
    std::unordered_map<const void*, ObjectRefPtr> m_objects;
    std::unordered_map<const void*, FunctionRefPtr> m_functions;
    std::unordered_map<const void*, LiteralRefPtr> m_globals;
    std::vector<LiteralRefPtr> m_literals;
    std::vector<KeyRefPtr> m_keys;
    std::vector<SetterRef> m_setters;

    template <class T>
    void sortRefsByDepth(const std::unordered_map<const void*, std::unique_ptr<T>>& in, std::list<T*>& out)
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
    Serializer(): m_root(0, false), m_true("true"), m_false("false"), m_nil("nil") {}

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
        KeyRef* keyRef = serializeKey(key, "");
        LiteralRef* valueRef = serializeLiteral(std::string("\"") + value + "\"");
        ref->setInlineRef(table, keyRef, valueRef);
    }

    void setBoolean(ObjectRef* ref, const std::string& table, const std::string& key, bool value)
    {
        KeyRef* keyRef = serializeKey(key, "");
        LiteralRef* valueRef = value ? &m_true : &m_false;
        ref->setInlineRef(table, keyRef, valueRef);
    }

    template <class T>
    void setNumber(ObjectRef* ref, const std::string& table, const std::string& key, T value)
    {
        KeyRef* keyRef = serializeKey(key, "");
        LiteralRef* valueRef = serializeLiteral(std::to_string(value));
        ref->setInlineRef(table, keyRef, valueRef);
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

        KeyRef* keyRef = serializeKey(key, "");
        LiteralRef* valueRef = serializeLiteral(str);
        ref->setInlineRef(table, keyRef, valueRef);
    }

    void populateGlobals(const std::string& prefix, lua_State* L, int index);

    void serializeSubtable(ObjectRef* parent, const std::string& table, lua_State* L, int index);
    void serializeSetter(const std::string& setter, lua_State* L, std::initializer_list<int> list);

    void serializeMember(ObjectRef* parent, const std::string& table, const std::string& key, const std::string& setter, lua_State* L, int index)
        {serializeMember(parent, table, serializeKey(key, setter), L, index);}

    void print();

private:
    LiteralRef* getGlobalRef(lua_State* L, int index);

    void serializeMember(ObjectRef* parent, const std::string& table, ILuaRef* key, lua_State* L, int index);

    KeyRef* serializeKey(const std::string& key, const std::string& setter)
    {
        KeyRef* ref = new KeyRef(key, setter);
        m_keys.emplace_back(ref);
        return ref;
    }

    LiteralRef* serializeLiteral(const std::string& literal)
    {
        LiteralRef* ref = new LiteralRef(literal);
        m_literals.emplace_back(ref);
        return ref;
    }

    LiteralRef* serializeNumber(lua_State* L, int index);

    ILuaRef* serializeValue(int depth, bool inlinable, lua_State* L, int index);
    ObjectRef* serializeObject(int depth, bool inlinable, lua_State* L, int index);
    FunctionRef* serializeFunction(int depth, lua_State* L, int index);
};
