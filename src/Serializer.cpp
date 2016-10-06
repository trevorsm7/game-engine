#include "Serializer.hpp"

#include "lua.hpp"
#include <algorithm>
#include <cassert>

using namespace std::string_literals;

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
    //sref.setter = setter.empty() ? "."s + key : ":"s + setter;
    ref.setter = setter;
    ref.value = value;

    // Can't inline objects referenced by name in a setter
    m_inlinable = false;
}

// TODO replace warnings with asserts
void Serializer::populateGlobals(const std::string& prefix, lua_State* L, int index)
{
    assert(lua_type(L, index) == LUA_TTABLE);

    // Hide the table itself (probably an error if it's visible somewhere)
    // NOTE this applies to recursively serialized __index tables as well
    m_globals[lua_topointer(L, index)] = std::string();

    // Iterate over table key/value pairs
    lua_pushnil(L);
    if (index < 0) --index; // adjust relative indices
    while (lua_next(L, index))
    {
        const void* ptr = lua_topointer(L, -1);

        // Skip non-reference types and break cycles
        if (ptr != nullptr || m_globals.find(ptr) != m_globals.end())
        {
            // TODO should we allow non-string keys?
            if (lua_type(L, -2) != LUA_TSTRING)
            {
                fprintf(stderr, "WARNING: global with %s key in %s\n", lua_typename(L, lua_type(L, -2)), prefix.c_str());
                lua_pop(L, 1);
                continue;
            }

            std::string key = prefix + lua_tostring(L, -2);
            m_globals[ptr] = key;

            // TODO recurse userdata (warning on tables?)
            if (lua_type(L, -1) == LUA_TUSERDATA)
            {
                int type = luaL_getmetafield(L, -1, "__index");
                if (type != LUA_TNIL)
                {
                    if (type == LUA_TTABLE)
                    {
                        populateGlobals(key + ".", L, -1);
                    }
                    else
                    {
                        fprintf(stderr, "WARNING: global userdata %s with %s index\n", key.c_str(), lua_typename(L, type));
                    }

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

        // Remove value, leaving key on top for next iteration of lua_next
        lua_pop(L, 1);
    }
}

std::string* Serializer::getGlobalName(lua_State* L, int index)
{
    const void* ptr = lua_topointer(L, index);

    if (ptr != nullptr)
    {
        auto it = m_globals.find(ptr);
        if (it != m_globals.end())
            return &it->second;
    }

    return nullptr;
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
        ILuaRef* key = serializeKey(parent->m_depth + 1, L, -2);
        // TODO need onStack/shared depths for functions so we can identify when to use a setter for function keys
        // TODO need to use setter for literals if the key is on stack!
        serializeMember(parent, table, key, L, -1);

        // Remove value, leaving key on top for next iteration of lua_next
        lua_pop(L, 1);
    }
}

void Serializer::serializeMember(ObjectRef* parent, const std::string& table, const std::string& key, const std::string& setter, lua_State* L, int index)
{
    if (parent == nullptr)
        parent = &m_root;

    const bool isRoot = (parent == &m_root);

    // Filter read-only globals (userdata, table, function)
    std::string* global = getGlobalName(L, index);
    if (global != nullptr)
    {
        if (!global->empty())
            setLiteral(parent, table, key, *global);
        return;
    }

    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        lua_pushvalue(L, index);
        setLiteral(parent, table, key, lua_tostring(L, -1));
        lua_pop(L, 1);
        break;
    case LUA_TSTRING:
        setString(parent, table, key, lua_tostring(L, index));
        break;
    case LUA_TBOOLEAN:
        setBoolean(parent, table, key, lua_toboolean(L, index));
        break;
    case LUA_TFUNCTION:
        {
            FunctionRef* ref = serializeFunction(parent->m_depth + 1, L, index);

            // Use a setter unless the function is global
            if (isRoot)
            {
                ref->setGlobalName(key);
                setLiteral(parent, table, key, ref->m_name); // NOTE only need to set this if key != m_name
            }
            else
            {
                setSetter(parent, key, setter, ref);
            }

            break;
        }
    case LUA_TTABLE:
    case LUA_TUSERDATA:
        {
            ObjectRef* ref = serializeObject(parent->m_depth + 1, true, L, index);

            // Break cycle if object is already on the stack
            if (ref->m_onStack || key.empty())
            {
                assert(!isRoot);
                // Force setter instead of inlining
                setSetter(parent, key, setter, ref);
                ref->m_inlinable = false;
            }
            else
            {
                // Add ref to parent
                LiteralRef* keyRef = serializeLiteral("."s + key);
                parent->setInlineRef(table, keyRef, ref);

                // Set name if object is global
                if (isRoot)
                    ref->setGlobalName(key);
            }

            break;
        }
    default:
        // TODO light userdata, thread? shouldn't ever encounter these
        fprintf(stderr, "can't set attrib %s: unsupported type %s\n", key.c_str(), lua_typename(L, type));
        break;
    }
}

// TODO how can we refactor this with the version that takes strings?
// Give LiteralRef 3 strings: key, setter, value??
void Serializer::serializeMember(ObjectRef* parent, const std::string& table, ILuaRef* key, lua_State* L, int index)
{
    if (parent == nullptr)
        parent = &m_root;

    const bool isRoot = (parent == &m_root);

    // Filter read-only globals (userdata, table, function)
    std::string* global = getGlobalName(L, index);
    if (global != nullptr)
    {
        if (!global->empty())
        {
            LiteralRef* value = serializeLiteral(*global);
            parent->setInlineRef(table, key, value);
        }
        return;
    }

    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        lua_pushvalue(L, index);
        parent->setInlineRef(table, key, serializeLiteral(lua_tostring(L, -1)));
        // TODO parent->setInlineRef(table, key, serializeNumber(L, -1))
        lua_pop(L, 1);
        break;
    case LUA_TSTRING:
        parent->setInlineRef(table, key, serializeLiteral("\""s + lua_tostring(L, index) + "\""));
        // TODO parent->setInlineRef(table, key, serializeString(L, index))
        break;
    case LUA_TBOOLEAN:
        parent->setInlineRef(table, key, serializeLiteral(lua_toboolean(L, index) ? "true" : "false"));
        // TODO parent->setInlineRef(table, key, serializeBoolean(L, index))
        break;
    case LUA_TFUNCTION:
        {
            FunctionRef* ref = serializeFunction(parent->m_depth + 1, L, index);

            // Use a setter unless the function is global
            if (isRoot)
            {
                ref->setGlobalName(key->getAsKey());
                parent->setInlineRef(table, key, ref);
            }
            else
            {
                parent->setSetterRef(key, ref);
            }

            break;
        }
    case LUA_TTABLE:
    case LUA_TUSERDATA:
        {
            ObjectRef* ref = serializeObject(parent->m_depth + 1, true, L, index);

            // Break cycle if object is already on the stack
            if (ref->m_onStack || key->isOnStack()) // TODO checking key on stack here is correct?
            {
                assert(!isRoot);
                // Force setter instead of inlining
                parent->setSetterRef(key, ref);
                ref->m_inlinable = false;
            }
            else
            {
                // Add ref to parent
                parent->setInlineRef(table, key, ref);

                // Set name if object is global
                if (isRoot)
                    ref->setGlobalName(key->getAsKey());
            }

            break;
        }
    default:
        // TODO light userdata, thread? shouldn't ever encounter these
        fprintf(stderr, "can't set attrib %s: unsupported type %s\n", key->getAsKey().c_str(), lua_typename(L, type));
        break;
    }
}

void Serializer::serializeUpvalue(FunctionRef* parent, lua_State* L, int index)
{
    assert(parent != nullptr);

    // Filter read-only globals (userdata, table, function)
    std::string* global = getGlobalName(L, index);
    if (global != nullptr)
    {
        if (!global->empty())
        {
            // TODO parent->setLiteral(table, key, it->second);
            parent->m_upvalues.emplace_back(serializeLiteral(*global));
        }
        return;
    }

    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        lua_pushvalue(L, index);
        parent->m_upvalues.emplace_back(serializeLiteral(lua_tostring(L, -1)));
        lua_pop(L, 1);
        break;
    case LUA_TSTRING:
        parent->m_upvalues.emplace_back(serializeLiteral("\""s + lua_tostring(L, index) + "\""));
        // TODO serializeString(L, index)
        break;
    case LUA_TBOOLEAN:
        parent->m_upvalues.emplace_back(serializeLiteral(lua_toboolean(L, index) ? "true" : "false"));
        // TODO serializeBoolean(L, index)
        break;
    case LUA_TNIL:
        parent->m_upvalues.emplace_back(serializeLiteral("nil"));
        break;
    case LUA_TFUNCTION:
        {
            FunctionRef* ref = serializeFunction(parent->m_depth + 1, L, index);

            // TODO move this to FunctionRef::setUpvalue
            // Make sure depth is less than all inline children
            if (parent->m_depth >= ref->m_depth)
                parent->m_depth = ref->m_depth - 1;

            parent->m_upvalues.emplace_back(ref);
            break;
        }
    case LUA_TTABLE:
    case LUA_TUSERDATA:
        parent->m_upvalues.emplace_back(serializeObject(parent->m_depth + 1, false, L, index));
        // TODO rebalance depth here if we collapse into same lists as functions
        break;
    default:
        // TODO light userdata, thread? shouldn't ever encounter these
        fprintf(stderr, "can't set upvalue: unsupported type %s\n", lua_typename(L, type));
        return;
    }
}

void Serializer::serializeSetter(const std::string& setter, lua_State* L, std::initializer_list<int> list)
{
    // Push a new ref at the back of the vector
    m_setters.emplace_back();
    SetterRef& ref = m_setters.back();
    ref.setter = setter;

    for (auto& index : list)
    {
        std::string* global = getGlobalName(L, index);
        if (global != nullptr)
        {
            // TODO will raise an error in print if global is restricted/empty
            ref.args.emplace_back(serializeLiteral(*global));
        }
        else
        {
            int type = lua_type(L, index);
            switch (type)
            {
            case LUA_TNUMBER:
                lua_pushvalue(L, index);
                ref.args.emplace_back(serializeLiteral(lua_tostring(L, -1)));
                lua_pop(L, 1);
                break;
            case LUA_TSTRING:
                ref.args.emplace_back(serializeLiteral("\""s + lua_tostring(L, index) + "\""));
                break;
            case LUA_TBOOLEAN:
                ref.args.emplace_back(serializeLiteral(lua_toboolean(L, index) ? "true" : "false"));
                break;
            case LUA_TNIL:
                ref.args.emplace_back(serializeLiteral("nil"));
                break;
            case LUA_TFUNCTION:
                ref.args.emplace_back(serializeFunction(0, L, index));
                break;
            case LUA_TTABLE:
            case LUA_TUSERDATA:
                ref.args.emplace_back(serializeObject(0, false, L, index));
                break;
            default:
                // TODO light userdata, thread? shouldn't ever encounter these
                fprintf(stderr, "can't set argument: unsupported type %s\n", lua_typename(L, type));
                break;
            }
        }
    }
}

ILuaRef* Serializer::serializeKey(int depth, lua_State* L, int index)
{
    ILuaRef* ptr = nullptr;

    // Filter read-only globals (userdata, table, function)
    std::string* global = getGlobalName(L, index);
    if (global != nullptr)
    {
        if (!global->empty())
            ptr = serializeLiteral(*global);

        return ptr;
    }

    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        lua_pushvalue(L, index);
        ptr = serializeLiteral(lua_tostring(L, -1));
        lua_pop(L, 1);
        break;
    case LUA_TSTRING:
        ptr = serializeLiteral("\""s + lua_tostring(L, index) + "\"");
        break;
    case LUA_TBOOLEAN:
        ptr = serializeLiteral(lua_toboolean(L, index) ? "true" : "false");
        break;
    case LUA_TFUNCTION:
        ptr = serializeFunction(depth, L, index);
        break;
    case LUA_TTABLE:
    case LUA_TUSERDATA:
        ptr = serializeObject(depth, false, L, index);
        // NOTE handle ptr->m_onStack in serializeMember
        break;
    default:
        fprintf(stderr, "unsupported table key type: %s", lua_typename(L, type));
        break;
    }

    return ptr;
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
        // Get userdata serialize function
        luaL_getmetafield(L, index, "serialize");
        assert(lua_type(L, -1) == LUA_TFUNCTION);

        // Push the userdata after the function
        const int adjIndex = (index < 0) ? index - 1 : index;
        lua_pushvalue(L, adjIndex);
        lua_pushlightuserdata(L, this);

        // Do a regular call; pops function and udata
        lua_call(L, 2, 0);
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

// TODO we could combine writer and format (elimate the vector) efficiently if we
// keep a bool to track if the previous char was an escaped number--if so, and if
// the next char is a digit, we can work back to the \ and add 0s until we hit 3
int vectorWriter(lua_State *L, const void* p, size_t sz, void* ud)
{
    auto data = reinterpret_cast<std::vector<char>*>(ud);
    auto chunk = reinterpret_cast<const char*>(p);
    data->insert(data->end(), chunk, chunk + sz);
    return 0;
}

void formatDump(const std::vector<char>& in, std::string& out)
{
    // adapted from private function 'addquoted' in lstrlib.c
    size_t size = in.size();
    const char* ptr = in.data();
    while (size--)
    {
        const char ch = *(ptr++);
        const unsigned char uch = (unsigned char)ch;
        const unsigned char unext = size > 0 ? (unsigned char)*ptr : '\0';
        if (ch == '"' || ch == '\\')// || ch == '\n')
        {
            out.push_back('\\');
            out.push_back(ch);
        }
        else if (ch == '\n')
        {
            out.push_back('\\');
            out.push_back('n');
        }
        else if (ch < ' ' || ch > '~')//iscntrl(uch)) // TODO keep UTF-8 codes or escape them?
        {
            char buffer[5];
            if (!isdigit(unext))
                snprintf(buffer, sizeof(buffer), "\\%d", int(uch));
            else
                snprintf(buffer, sizeof(buffer), "\\%03d", int(uch));
            out += buffer;
        }
        else
        {
            out.push_back(ch);
        }
    }
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

    std::vector<char> data;
    lua_dump(L, vectorWriter, &data, true); // func must be on top of stack
    formatDump(data, ref->m_code);

    for (int i = 1; true; ++i)
    {
        if (!lua_getupvalue(L, index, i))
            break;

        serializeUpvalue(ref, L, -1);

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
        ref->print(this, 0, false);
        printf("\n");
    }

    // Sort function refs into list by ref depth, descending order
    std::list<FunctionRef*> functionsSorted;
    sortRefsByDepth(m_functions, functionsSorted);

    // TODO handle duplicated globals in printInlines
    // TODO refactor with objectsSorted
    for (auto& ref : functionsSorted)
    {
        // TODO make sure there are no name clashes; can just use any unique name
        if (ref->m_tempName)
            ref->m_name = "_temp"s + std::to_string(tempIndex++);

        printf("%s%s = ", (ref->m_tempName ? "local " : ""), ref->m_name.c_str());
        ref->print(this, 0, false);
        printf("\n");
    }

    // Iterate over setters
    for (auto& parent : objectsSorted)
        printSetters(parent);

    // Print global inlines
    // NOTE must be after functions since root inlines can reference them by name
    printInlines(&m_root, 0);

    // Iterate over global setters
    for (auto& ref : m_setters)
    {
        printf("%s(", ref.setter.c_str());
        //for (auto& arg : ref.args)
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
}

void Serializer::printSetters(const ObjectRef* ref)
{
    assert(ref != nullptr);

    for (auto& sref : ref->m_setters)
    {
        std::string setter = sref.setter->getAsSetter();
        std::string value = sref.value->getAsValue();
        if (setter.empty() || value.empty())
        {
            fprintf(stderr, "warning: empty setter/value in %s - (%s, %s)\n", ref->m_name.c_str(), setter.c_str(), value.c_str());
            continue;
        }

        if (setter[0] == ':')
            printf("%s%s(%s)\n", ref->m_name.c_str(), setter.c_str(), value.c_str());
        else
            printf("%s%s = %s\n", ref->m_name.c_str(), setter.c_str(), value.c_str());
    }
}

void Serializer::printInlines(const ObjectRef* ref, int indent)
{
    assert(ref != nullptr);
    const bool isRoot = (ref == &m_root);

    bool firstLine = true;
    std::string lastTable;

    for (auto& iref : ref->m_inlines)
    {
        std::string key = iref.key->getAsKey();
        assert(!key.empty());

        // Skip global objects that have already been printed
        // NOTE will always be true unless more than one global points at the object
        // TODO HACK removing redundant function/global inlines here (currently stored as literal)
        if (isRoot && (key == iref.value->getAsValue()))
            continue;

        // Handle change of subtable formatting
        bool tableChanged = (lastTable != iref.table);

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
            printf("%*s%s =\n%*s{\n", indent, "", iref.table.c_str(), indent, "");
            indent += 2;

            lastTable = iref.table;
        }

        // Print the next line
        if (isRoot && key.front() == '[')
            printf("%*s_G%s = ", indent, "", key.c_str());
        else
            printf("%*s%s = ", indent, "", key.c_str());
        iref.value->print(this, indent, true);
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

void ObjectRef::print(Serializer* serializer, int indent, bool isInline) const
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
            serializer->printInlines(this, indent + 2); // TODO move this into ObjectRef too!
            printf("%*s}", indent, "");
        }
        else
        {
            printf("{}");
        }
    }
}
