#include "Serializer.hpp"
#include "IUserdata.hpp"

#include "lua.hpp"
#include <algorithm>
#include <cassert>

using namespace std::string_literals;

std::string KeyRef::getAsSetter() const
{
    if (m_setter.empty())
    {
        assert(!m_key.empty());
        return "."s + m_key;
    }

    return ":"s + m_setter;
}

std::string LiteralRef::getAsKey() const
{
    assert(!m_literal.empty());

    if (m_literal.front() == '\"' && m_literal.back() == '\"')
        return m_literal.substr(1, m_literal.size() - 2);

    return "["s + m_literal + "]";
}

std::string LiteralRef::getAsSetter() const
{
    assert(!m_literal.empty());

    if (m_literal.front() == '\"' && m_literal.back() == '\"')
        return "."s + m_literal.substr(1, m_literal.size() - 2);

    return "["s + m_literal + "]";
}

bool FunctionRef::setGlobalName(const ILuaRef* key)
{
    // Only set name the first time this is called
    std::string name = key->getAsKey();
    if (m_tempName && !name.empty() && name.front() != '[')
    {
        m_name = name;
        m_tempName = false;
        return true;
    }
    return false;
}

bool ObjectRef::setGlobalName(const ILuaRef* key)
{
    // Only set name the first time this is called
    std::string name = key->getAsKey();
    if (m_tempName && !name.empty() && name.front() != '[')
    {
        m_name = name;
        m_inlinable = false;
        m_tempName = false;
        return true;
    }
    return false;
}

void ObjectRef::setInlineRef(const std::string& table, ILuaRef* key, ILuaRef* value)
{
    // Alphabetical search from the end by subtable name
    auto it = std::find_if(m_inlines.rbegin(), m_inlines.rend(),
        [&table](InlineRef& ref) {return table.compare(ref.table) >= 0;});

    // Insert alphabetically after entires with same subtable name
    auto ref = m_inlines.emplace(it.base());
    ref->table = table;
    ref->key = key;
    ref->value = value;

    // Make sure depth is less than the refs
    const int keyDepth = key->getDepth();
    if (m_depth >= keyDepth)
        m_depth = keyDepth - 1;

    const int valueDepth = value->getDepth();
    if (m_depth >= valueDepth)
        m_depth = valueDepth - 1;
}

void ObjectRef::setSetterRef(ILuaRef* setter, ILuaRef* value)
{
    // Push setter reference
    m_setters.emplace_back();
    SetterRef& ref = m_setters.back();
    ref.setter = setter;
    ref.value = value;

    // Can't inline objects referenced by name in a setter
    m_inlinable = false;
}

void Serializer::populateGlobals(const void* G, const std::string& prefix, lua_State* L, int index)
{
    assert(lua_type(L, index) == LUA_TTABLE);

    // Hide the table itself (probably an error if it's visible somewhere)
    // NOTE this applies to recursively serialized __index tables as well
    // TODO can we just remove this? we already boned if user can touch these
    m_globals[lua_topointer(L, index)] = LiteralRefPtr(new LiteralRef("{}"));

    // Iterate over table key/value pairs
    lua_pushnil(L);
    if (index < 0) --index; // adjust relative indices
    while (lua_next(L, index))
    {
        const void* ptr = lua_topointer(L, -1);

        // Skip non-reference types and break cycles
        if (ptr != nullptr && m_globals.find(ptr) == m_globals.end())
        {
            // TODO should we allow non-string keys?
            assert(lua_type(L, -2) == LUA_TSTRING);
            std::string key = prefix + lua_tostring(L, -2);
            m_globals[ptr] = LiteralRefPtr(new LiteralRef(key));

            if (ptr != G)
            {
                if (lua_type(L, -1) == LUA_TUSERDATA)
                {
                    if (luaL_getmetafield(L, -1, "__index") != LUA_TNIL)
                    {
                        assert(lua_type(L, -1) == LUA_TTABLE);
                        populateGlobals(G, key + ".", L, -1);
                        lua_pop(L, 1);
                    }
                }
                else if (lua_type(L, -1) == LUA_TTABLE)
                {
                    fprintf(stderr, "WARNING: global table %s is mutable\n", key.c_str());
                    // NOTE the table itself can be treated as a const global, but it's children cannot
                    //populateGlobals(key + ".", L, -1);
                }
            }
        }

        // Remove value, leaving key on top for next iteration of lua_next
        lua_pop(L, 1);
    }
}

// TODO possible to overflow the Lua stack if we have a chain of tables; break this up with lua_call as with userdata?
void Serializer::serializeSubtable(ObjectRef* parent, const std::string& table, lua_State* L, int index)
{
    assert(lua_type(L, index) == LUA_TTABLE);

    if (parent == nullptr)
        parent = &m_root;

    // Iterate over table key/value pairs
    lua_pushnil(L);
    if (index < 0) --index; // adjust relative indices
    while (lua_next(L, index))
    {
        ILuaRef* key = serializeValue(parent->m_depth + 1, false, L, -2);
        assert(key != nullptr);
        serializeMember(parent, table, key, L, -1);

        // Remove value, leaving key on top for next iteration of lua_next
        lua_pop(L, 1);
    }
}

void Serializer::serializeMember(ObjectRef* parent, const std::string& table, const std::string& key, const std::string& setter, lua_State* L, IUserdata* member)
{
    if (member != nullptr)
    {
        member->pushUserdata(L);
        serializeMember(parent, table, serializeKey(key, setter), L, -1);
        lua_pop(L, 1);
    }
}

void Serializer::serializeMember(ObjectRef* parent, const std::string& table, ILuaRef* key, lua_State* L, int index)
{
    if (parent == nullptr)
        parent = &m_root;

    ILuaRef* ref = serializeValue(parent->m_depth + 1, !key->isSetterOnly(), L, index);
    assert(ref != nullptr);

    if (parent == &m_root)
    {
        if (!ref->setGlobalName(key))
            parent->setInlineRef(table, key, ref);
    }
    else
    {
        if (ref->isSetterOnly() || key->isSetterOnly())
        {
            parent->setSetterRef(key, ref);
        }
        else
        {
            parent->setInlineRef(table, key, ref);
        }
    }
}

void Serializer::serializeSetter(const std::string& setter, lua_State* L, std::initializer_list<int> list)
{
    // Push a new ref at the back of the vector
    m_setters.emplace_back();
    SetterRef& setterRef = m_setters.back();
    setterRef.setter = setter;

    for (auto& index : list)
    {
        ILuaRef* ref = serializeValue(0, false, L, index);
        assert(ref != nullptr);
        setterRef.args.emplace_back(ref);
    }
}

LiteralRef* Serializer::serializeNumber(lua_State* L, int index)
{
    lua_pushvalue(L, index);
    LiteralRef* ref = serializeLiteral(lua_tostring(L, -1));
    // if (!lua_isinteger(L, index) string.unpack(\"<f\",") + ??
    lua_pop(L, 1);
    return ref;
}

ILuaRef* Serializer::serializeValue(int depth, bool inlinable, lua_State* L, int index)
{
    const void* ptr = lua_topointer(L, index);
    if (ptr != nullptr)
    {
        auto it = m_globals.find(ptr);
        if (it != m_globals.end())
            return it->second.get();
    }

    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        return serializeNumber(L, index);
    case LUA_TSTRING:
        return serializeLiteral("\""s + lua_tostring(L, index) + "\"");
    case LUA_TBOOLEAN:
        return lua_toboolean(L, index) ? &m_true : &m_false;
    case LUA_TNIL:
        return &m_nil;
    case LUA_TFUNCTION:
        return serializeFunction(depth, L, index);
    case LUA_TTABLE:
    case LUA_TUSERDATA:
        return serializeObject(depth, inlinable, L, index);
    default:
        // TODO light userdata, thread? shouldn't ever encounter these
        fprintf(stderr, "unsupported type: %s", lua_typename(L, type));
        return nullptr;
    }
}

ObjectRef* Serializer::serializeObject(int depth, bool inlinable, lua_State* L, int index)
{
    // Get userdata pointer
    const void* ptr = lua_topointer(L, index);
    assert(ptr != nullptr);

    // If ref already exists, return it
    ObjectRef* ref = getObjectRef(ptr);
    if (ref != nullptr)
    {
        ref->m_inlinable = false;
        return ref;
    }

    // Create a new object ref
    ref = new ObjectRef(depth, inlinable);
    m_objects[ptr] = ObjectRefPtr(ref); // take ownership of bare ptr

    const int type = lua_type(L, index);
    assert(type == LUA_TUSERDATA || type == LUA_TTABLE);

    // Handle userdata and tables separately
    if (type == LUA_TUSERDATA)
    {
        int top = lua_gettop(L);

        // Set constructor to the userdata class name
        luaL_getmetafield(L, index, "__metatable");
        assert(lua_gettop(L) > top && lua_type(L, -1) == LUA_TSTRING);
        ref->m_constructor = lua_tostring(L, -1);
        lua_settop(L, top);

        // Get userdata serialize function
        luaL_getmetafield(L, index, "serialize");
        assert(lua_gettop(L) > top && lua_type(L, -1) == LUA_TFUNCTION);

        // Push the userdata after the function
        const int adjIndex = (index < 0) ? index - 1 : index;
        lua_pushvalue(L, adjIndex);
        lua_pushlightuserdata(L, this);
        lua_pushlightuserdata(L, ref);

        // Do a regular call; pops function and udata
        lua_call(L, 3, 0);
        assert(lua_gettop(L) == top);
    }
    else
    {
        serializeSubtable(ref, "", L, index);

        // Serialize the metatable, if it exists
        if (lua_getmetatable(L, index))
        {
            assert(lua_type(L, -1) == LUA_TTABLE);
            const int adjIndex = (index < 0) ? index - 1 : index;
            serializeSetter("setmetatable", L, {adjIndex, -1});
            lua_pop(L, 1);
        }
    }

    // Clear cycle detection flag before returning
    ref->m_onStack = false;

    return ref;
}

static int stringWriter(lua_State *L, const void* ptr, size_t size, void* user)
{
    auto data = reinterpret_cast<std::string*>(user);
    auto chunk = reinterpret_cast<const char*>(ptr);

    bool escapeDigit = (data->size() > 0 && isdigit(data->back()));

    while (size--)
    {
        const char ch = *(chunk++);
        const unsigned char uch = (unsigned char)ch;

        if (ch == '"' || ch == '\\')
        {
            data->push_back('\\');
            data->push_back(ch);
            escapeDigit = false;
        }
        else if (ch == '\n')
        {
            data->push_back('\\');
            data->push_back('n');
            escapeDigit = false;
        }
        else if (ch < ' ' || ch > '~' || (isdigit(ch) && escapeDigit))
        //else if (iscntrl(uch) || (isdigit(ch) && escapeDigit))
        {
            char buffer[5];
            int n = snprintf(buffer, sizeof(buffer), "\\%d", int(uch));
            escapeDigit = (n < 4);
            data->append(buffer);
        }
        else
        {
            data->push_back(ch);
            escapeDigit = false;
        }
    }

    return 0;
}

FunctionRef* Serializer::serializeFunction(int depth, lua_State* L, int index)
{
    assert(lua_type(L, index) == LUA_TFUNCTION);

    // Get function pointer and data store
    const void* ptr = lua_topointer(L, index);
    assert(ptr != nullptr);

    FunctionRef* ref = getFunctionRef(ptr);
    if (ref != nullptr)
        return ref;

    // Create a new function ref
    ref = new FunctionRef(depth);
    m_functions[ptr] = FunctionRefPtr(ref); // take ownership of bare ptr

    // NOTE lua_dump expected func on top of stack
    lua_pushvalue(L, index);
    lua_dump(L, stringWriter, &ref->m_code, true);
    lua_pop(L, 1);

    // Serialize upvalues
    for (int i = 1; true; ++i)
    {
        if (!lua_getupvalue(L, index, i))
            break;

        ILuaRef* upvalue = serializeValue(ref->m_depth + 1, false, L, -1);
        assert(upvalue != nullptr);

        const int depth = upvalue->getDepth();
        if (ref->m_depth >= depth)
            ref->m_depth = depth - 1;

        ref->m_upvalues.emplace_back(upvalue);

        lua_pop(L, 1);
    }

    return ref;
}

void Serializer::print()
{
    // TODO write to buffer/file instead of printf

    int tempIndex = 1;

    // Sort object refs into list by ref depth, descending order
    std::list<ObjectRef*> objectsSorted;
    sortRefsByDepth(m_objects, objectsSorted);

    for (auto& ref : objectsSorted)
    {
        // Skip inlinable objects
        if (ref->m_inlinable)
            continue;

        // TODO make sure there are no name clashes; can just use any unique name
        if (ref->m_tempName)
            ref->m_name = "_temp"s + std::to_string(tempIndex++);

        // Print object
        printf("%s%s = ", (ref->m_tempName ? "local " : ""), ref->m_name.c_str());
        ref->print(0, false);
        printf("\n");
    }

    // Sort function refs into list by ref depth, descending order
    std::list<FunctionRef*> functionsSorted;
    sortRefsByDepth(m_functions, functionsSorted);

    // TODO refactor with objectsSorted
    for (auto& ref : functionsSorted)
    {
        // TODO make sure there are no name clashes; can just use any unique name
        if (ref->m_tempName)
            ref->m_name = "_temp"s + std::to_string(tempIndex++);

        printf("%s%s = ", (ref->m_tempName ? "local " : ""), ref->m_name.c_str());
        ref->print(0, false);
        printf("\n");
    }

    // Iterate over setters
    for (auto& parent : objectsSorted)
        parent->printSetters();

    // Print global inlines
    // NOTE must be after functions since root inlines can reference them by name
    m_root.printInlines(0, true);

    // Iterate over global setters
    for (auto& ref : m_setters)
    {
        printf("%s(", ref.setter.c_str());
        if (!ref.args.empty())
        {
            auto it = ref.args.begin();
            printf("%s", (*it)->getAsValue().c_str());

            auto end = ref.args.end();
            while (++it != end)
                printf(", %s", (*it)->getAsValue().c_str());
        }
        printf(")\n");
    }

    if (m_env != nullptr)
        printf("_ENV = %s\n", m_env->getAsValue().c_str());
}

void FunctionRef::print(int indent, bool isInline) const
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

void ObjectRef::printSetters() const
{
    for (auto& ref : m_setters)
    {
        std::string setter = ref.setter->getAsSetter();
        std::string value = ref.value->getAsValue();
        assert(!setter.empty() && !value.empty());

        if (setter[0] == ':')
            printf("%s%s(%s)\n", m_name.c_str(), setter.c_str(), value.c_str());
        else
            printf("%s%s = %s\n", m_name.c_str(), setter.c_str(), value.c_str());
    }
}

void ObjectRef::printInlines(int indent, bool isRoot) const
{
    bool firstLine = true;
    std::string lastTable;

    for (auto& ref : m_inlines)
    {
        // Handle change of subtable formatting
        bool tableChanged = (lastTable != ref.table);

        // Close previous subtable
        if (tableChanged && !lastTable.empty())
        {
            indent -= 2;
            printf("\n%*s}", indent, "");
        }

        // Handle end of line formatting
        if (!firstLine)
            printf(isRoot && (lastTable.empty() || tableChanged) ? "\n" : ",\n");

        firstLine = false;

        // Open new subtable
        if (tableChanged)
        {
            printf("%*s%s =\n%*s{\n", indent, "", ref.table.c_str(), indent, "");
            indent += 2;

            lastTable = ref.table;
        }

        // Print the next line
        std::string key = ref.key->getAsKey();
        assert(!key.empty());
        if (isRoot && key.front() == '[')
            printf("%*s_G%s = ", indent, "", key.c_str());
        else
            printf("%*s%s = ", indent, "", key.c_str());
        ref.value->print(indent, true);
    }

    // Close the subtable
    if (!lastTable.empty())
    {
        indent -= 2;
        printf("\n%*s}", indent, "");
    }

    if (!firstLine)
        printf("\n");
}

void ObjectRef::print(int indent, bool isInline) const
{
    if (isInline && !m_inlinable)
    {
        printf("%s", m_name.c_str());
    }
    else
    {
        printf("%s", m_constructor.c_str());

        if (!m_inlines.empty())
        {
            printf("\n%*s{\n", indent, "");
            printInlines(indent + 2, false);
            printf("%*s}", indent, "");
        }
        else
        {
            printf("{}");
        }
    }
}
