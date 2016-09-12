#include "Serializer.hpp"

#include <list>
#include <algorithm>

using namespace std::string_literals;

void ObjectRef::setLiteralRaw(std::string table, std::string key, std::string value)
{
    // Alphabetical search from the end by subtable name
    auto it = std::find_if(m_inlines.rbegin(), m_inlines.rend(),
        [&table](InlineRef& iref) {return table.compare(iref.table) >= 0;});

    // Insert alphabetically after entires with same subtable name
    InlineRef iref = {table, key, value, nullptr};
    m_inlines.insert(it.base(), iref);
}

void ObjectRef::setInlineRef(std::string table, std::string key, ObjectRef* ref)
{
    // Alphabetical search from the end by subtable name
    auto it = std::find_if(m_inlines.rbegin(), m_inlines.rend(),
        [&table](InlineRef& iref) {return table.compare(iref.table) >= 0;});

    // Insert alphabetically after entires with same subtable name
    InlineRef iref = {table, key, "", ref};
    m_inlines.insert(it.base(), iref);
}

void ObjectRef::setSetterRef(std::string key, std::string setter, ObjectRef* ref, bool isRoot)
{
    m_inlinable = false;

    if (isRoot)
    {
        assert(!setter.empty());
        SetterRef sref = {setter, ref, nullptr};
        m_setters.push_back(sref);
    }
    else if (!setter.empty())
    {
        SetterRef sref = {":"s + setter, ref, nullptr};
        m_setters.push_back(sref);
    }
    else
    {
        assert(!key.empty());
        SetterRef sref = {"."s + key, ref, nullptr};
        m_setters.push_back(sref);
    }
}

ObjectRef* Serializer::addObjectRef(const void* ptr, int depth)
{
    assert(getObjectRef(ptr) == nullptr);
    ObjectRef* ref = new ObjectRef(depth);
    m_objectRefs[ptr] = ObjectRefPtr(ref); // take ownership of bare ptr
    return ref;
}

ObjectRef* Serializer::getObjectRef(const void* ptr)
{
    auto it = m_objectRefs.find(ptr);
    if (it == m_objectRefs.end())
        return nullptr;

    return it->second.get();
}

// TODO replace warnings with asserts
void Serializer::populateGlobals(std::string prefix, lua_State* L, int index)
{
    assert(lua_type(L, index) == LUA_TTABLE);

    // Iterate over table key/value pairs
    lua_pushnil(L);
    if (index < 0) --index; // adjust relative indices
    while (lua_next(L, index))
    {
        const void* ptr = lua_topointer(L, -1);

        // Skip non-reference types and break cycles
        // NOTE make sure no globals with same function but different upvalues
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
                //populateGlobals(key + ".", L, -1);
            }
        }

        // Remove value, leaving key on top for next iteration of lua_next
        lua_pop(L, 1);
    }
}

void Serializer::serializeValue(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index)
{
    if (parent == nullptr)
        parent = &m_root;

    // Filter read-only globals (userdata, table, function)
    // TODO allow user to provide filters as well - would need to defer to serializeObject/Function then!
    const void* ptr = lua_topointer(L, index);
    if (ptr != nullptr)
    {
        auto it = m_globals.find(ptr);
        if (it != m_globals.end())
        {
            // Use the global name directly
            parent->setLiteralRaw(table, key, it->second);
            return;
        }
    }

    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        // TODO consider using setLiteral - need to check if integer or float
        // NOTE this will convert the value on the stack, so push a copy first
        lua_pushvalue(L, index);
        parent->setLiteralRaw(table, key, lua_tostring(L, -1));
        lua_pop(L, 1);
        break;
    case LUA_TSTRING:
        parent->setLiteral(table, key, lua_tostring(L, index));
        break;
    case LUA_TBOOLEAN:
        parent->setLiteral(table, key, lua_toboolean(L, index));
        break;
    case LUA_TFUNCTION:
        serializeFunction(parent, table, key, setter, L, index);
        break;
    case LUA_TTABLE:
    case LUA_TUSERDATA:
        serializeObject(parent, table, key, setter, L, index);
        break;
    default:
        // TODO light userdata, thread? shouldn't ever encounter these
        fprintf(stderr, "can't set attrib %s: unsupported type %s,\n", key, lua_typename(L, type));
        break;
    }
}

void Serializer::serializeObject(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index)
{
    if (parent == nullptr)
        parent = &m_root;

    const bool isRoot = (parent == &m_root);

    // Get userdata pointer and data store
    const void* ptr = lua_topointer(L, index);
    assert(ptr != nullptr);
    ObjectRef* ref = getObjectRef(ptr);

    // Check if object ref already exists
    if (ref)
    {
        ref->m_inlinable = false;

        // Break cycle if object is already on the stack
        if (ref->m_onStack)
        {
            parent->setSetterRef(key, setter, ref, isRoot);
            return;
        }
    }
    else
    {
        // Create a new object ref
        const int depth = parent->m_depth + 1;
        ref = addObjectRef(ptr, depth);
        assert(ref != nullptr);

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
            serializeFromTable(ref, "", L, index);
    }

    if (key[0] == '\0')
    {
        // Force setter instead of inlining
        parent->setSetterRef(key, setter, ref, isRoot);
        ref->m_inlinable = false;
    }
    else
    {
        // Rebalance parent depth
        if (parent->m_depth >= ref->m_depth)
            parent->m_depth = ref->m_depth - 1;

        // Add ref to parent
        parent->setInlineRef(table, key, ref);

        // Set name if object is global
        if (isRoot)
            ref->setGlobalName(key);
    }

    // Clear cycle detection flag before returning
    ref->m_onStack = false;
}

void Serializer::serializeFunction(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index)
{
    // TODO serialize binary dump? don't forget upvalues
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "<TODO: function %p>", lua_topointer(L, index));
    parent->setLiteralRaw(table, key, buffer);
}

// TODO possible to overflow the Lua stack if we have a chain of tables; break this up with lua_call as with userdata?
void Serializer::serializeFromTable(ObjectRef* ref, const char* table, lua_State* L, int index)
{
    assert(lua_type(L, index) == LUA_TTABLE);

    if (ref == nullptr)
        ref = &m_root;

    // Iterate over table key/value pairs
    lua_pushnil(L);
    if (index < 0) --index; // adjust relative indices
    while (lua_next(L, index))
    {
        // Handle different key types
        int type = lua_type(L, -2);
        switch (type)
        {
        case LUA_TNUMBER:
            {
                // NOTE this will convert the value on the stack, so push a copy first
                lua_pushvalue(L, -2);
                std::string key = "["s + lua_tostring(L, -1) + "]";
                lua_pop(L, 1);
                serializeValue(ref, table, key.c_str(), "", L, -1);
            }
            break;
        case LUA_TSTRING:
            serializeValue(ref, table, lua_tostring(L, -2), "", L, -1);
            break;
        case LUA_TBOOLEAN:
            serializeValue(ref, table, lua_toboolean(L, -2) ? "[true]" : "[false]", "", L, -1);
            break;
        // TODO handling these cases may be a little trickier
        case LUA_TFUNCTION:
        case LUA_TTABLE:
        case LUA_TUSERDATA:
        default:
            fprintf(stderr, "unsupported table key type: %s", lua_typename(L, type));
            break;
        }

        // Remove value, leaving key on top for next iteration of lua_next
        lua_pop(L, 1);
    }
}

void Serializer::print()
{
    // TODO clean this up!
    // TODO write to buffer/file instead of printf

    std::list<ObjectRef*> ordered;

    // Sort object refs into list by ref depth, descending order
    for (auto& pair : m_objectRefs)
    {
        // Find first index with lower ref depth
        int depth = pair.second->m_depth;
        auto it = std::find_if(ordered.begin(), ordered.end(),
            [depth](ObjectRef* ptr) {return ptr->m_depth < depth;});

        // Insert before index with lower ref depth
        ordered.insert(it, pair.second.get());
    }

    int index = 1;
    for (auto& ref : ordered)
    {
        // Skip inlinable objects
        if (ref->m_inlinable)
            continue;

        // TODO make sure there are no name clashes; can just use any unique name
        if (ref->m_tempName)
            ref->m_name = "_temp"s + std::to_string(index++);

        // Print object
        printf("%s%s = ", (ref->m_tempName ? "local " : ""), ref->m_name.c_str());
        printObject(ref, 0);
        printf("\n");
    }

    // Print global inlines
    printInlines(&m_root, 0);

    // TODO print functions and closures

    // Iterate over setters
    for (auto& parent : ordered)
        printSetters(parent);

    printSetters(&m_root);
}

void Serializer::printSetters(ObjectRef* ref)
{
    assert(ref != nullptr);
    for (auto& sref : ref->m_setters)
    {
        // Assert exactly one non-null
        assert(!sref.object != !sref.function);

        const char* name;
        if (sref.object != nullptr)
            name = sref.object->m_name.c_str();
        else
            name = sref.function->m_name.c_str();

        // Use different formatting for setter functions
        if (sref.setter[0] == '.')
            printf("%s%s = %s\n", ref->m_name.c_str(), sref.setter.c_str(), name);
        else //if (sref.setter[0] == ':') // or global (empty parent name, no separator)
            printf("%s%s(%s)\n", ref->m_name.c_str(), sref.setter.c_str(), name);
    }
}

void Serializer::printInlines(ObjectRef* ref, int indent)
{
    assert(ref != nullptr);
    const bool isRoot = (ref == &m_root);

    bool firstLine = true;
    std::string lastTable;

    for (auto& iref : ref->m_inlines)
    {
        // Skip global objects that have already been printed
        // NOTE will always be true unless more than one global points at the object
        if (iref.object && isRoot && iref.key == iref.object->m_name)
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
            printf(((!lastTable.empty() && !tableChanged) || isRoot) ? ",\n" : "\n");

        firstLine = false;

        // Open new subtable
        if (tableChanged)
        {
            printf("%*s%s =\n%*s{\n", indent, "", iref.table.c_str(), indent, "");
            indent += 2;

            lastTable = iref.table;
        }

        // Print the next line
        if (iref.object)
        {
            if (iref.object->m_inlinable)
            {
                // Handle nested objects recursively
                printf("%*s%s = ", indent, "", iref.key.c_str());
                printObject(iref.object, indent);
            }
            else
            {
                printf("%*s%s = %s", indent, "", iref.key.c_str(), iref.object->m_name.c_str());
            }
        }
        else // literal
        {
            printf("%*s%s = %s", indent, "", iref.key.c_str(), iref.literal.c_str());
        }
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

void Serializer::printObject(ObjectRef* ref, int indent)
{
    assert(ref != nullptr);
    printf("%s", ref->m_constructor.c_str());

    if (!ref->m_inlines.empty())
    {
        printf("\n%*s{\n", indent, "");
        printInlines(ref, indent + 2);
        printf("%*s}", indent, "");
    }
    else
    {
        printf("{}");
    }
}
