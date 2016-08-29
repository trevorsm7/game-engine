#include "Serializer.hpp"

#include <list>
#include <algorithm>

using namespace std::string_literals;

template <>
void ObjectRef::setType<bool>(std::string table, std::string key, bool value)
{
    setImmediate(table, key, value ? "true"s : "false"s);
}

template <>
void ObjectRef::setType<const char*>(std::string table, std::string key, const char* value)
{
    std::string str = "\""s + value + "\""s;
    setImmediate(table, key, str);
}

void ObjectRef::setImmediate(std::string table, std::string key, std::string value)
{
    // Alphabetical search from the end by subtable name
    auto it = std::find_if(m_immediates.rbegin(), m_immediates.rend(),
        [&table](ImmediateVal& val) {return table.compare(val.table) >= 0;});

    // Insert alphabetically after entires with same subtable name
    ImmediateVal val = {table, key, value};
    m_immediates.insert(it.base(), val);
    //m_immediates.emplace_back(table, name, value);
}

void ObjectRef::setObjectDirect(std::string table, std::string key, const void* ptr)
{
    // Alphabetical search from the end by subtable name
    auto it = std::find_if(m_directObjs.rbegin(), m_directObjs.rend(),
        [&table](ObjectDirect& val) {return table.compare(val.table) >= 0;});

    // Insert alphabetically after entires with same subtable name
    ObjectDirect val = {table, key, ptr};
    m_directObjs.insert(it.base(), val);
    //m_directObjs.emplace_back(table, key, ptr);
}

void ObjectRef::setObjectCycle(std::string key, std::string setter, const void* ptr)
{
    // TODO use a flag instead?
    std::string fullName = std::string(1, ':') + setter;
    if (fullName.length() == 1)
        fullName = std::string(1, '.') + key;
    ObjectCycle val = {fullName, ptr};
    m_cyclicObjs.push_back(val);
    //m_cyclicObjs.emplace_back(key, ptr);
}

ObjectRef* Serializer::addObjectRef(const void* ptr, int depth)
{
    assert(getObjectRef(ptr) == nullptr);
    ObjectRef* ref = new ObjectRef(depth);
    //m_objectRefs.emplace(userdata, ptr);
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

void Serializer::serializeValue(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index)
{
    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        // NOTE this will convert the value on the stack, so push a copy first
        lua_pushvalue(L, index);
        parent->setImmediate(table, key, lua_tostring(L, -1));
        lua_pop(L, 1);
        break;
    case LUA_TSTRING:
        {
            std::string str = std::string("\"") + lua_tostring(L, index) + "\"";
            parent->setImmediate(table, key, str);
        }
        break;
    case LUA_TBOOLEAN:
        parent->setImmediate(table, key, lua_toboolean(L, index) ? "true" : "false");
        break;
    case LUA_TFUNCTION:
        {
            // TODO serialize binary dump? don't forget upvalues
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "<TODO: function %p>", lua_topointer(L, index));
            parent->setImmediate(table, key, buffer);
        }
        break;
    case LUA_TTABLE:
    case LUA_TUSERDATA:
        serializeObject(parent, table, key, setter, L, index);
        break;
    default:
        // NOTE light userdata, thread?
        printf("can't set attrib %s: unsupported type %s,\n", key, lua_typename(L, type));
        break;
    }
}

void Serializer::serializeObject(ObjectRef* parent, const char* table, const char* key, const char* setter, lua_State* L, int index)
{
    // Get userdata pointer and data store
    const void* ptr = lua_topointer(L, index);
    assert(ptr != nullptr);
    ObjectRef* ref = getObjectRef(ptr);

    // Check if object ref already exists
    if (ref)
    {
        ref->m_count++;

        // Break cycle if object is already on the stack
        if (ref->m_onStack)
        {
            if (parent)
                parent->setObjectCycle(key, setter, ptr);
            else // HACK
                m_root = ref;

            return;
        }
    }
    else
    {
        // Create a new object ref
        const int depth = parent ? (parent->m_depth + 1) : 0;
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

    if (parent)
    {
        // Rebalance parent depth
        if (parent->m_depth >= ref->m_depth)
            parent->m_depth = ref->m_depth - 1;

        // Add ref to parent
        parent->setObjectDirect(table, key, ptr);
    }
    else // HACK
        m_root = ref;

    // Clear the onStack flag
    ref->m_onStack = false;
}

// TODO possible to overflow the Lua stack if we have a chain of tables; break this up with lua_call as with userdata?
void Serializer::serializeFromTable(ObjectRef* ref, const char* table, lua_State* L, int index)
{
    assert(lua_type(L, index) == LUA_TTABLE);

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
                std::string key = std::string("[") + lua_tostring(L, -1) + "]";
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
            printf("unsupported table key type: %s", lua_typename(L, type));
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
        // Reset index before ordering
        pair.second->m_index = 0;

        // Find first index with lower ref depth
        int depth = pair.second->m_depth;
        auto it = std::find_if(ordered.begin(), ordered.end(),
            [depth](ObjectRef* ptr) {return ptr->m_depth < depth;});

        // Insert before index with lower ref depth
        ordered.insert(it, pair.second.get());
    }

    printf("temp = {}\n");

    int index = 1;
    for (auto& ref : ordered)
    {
        // Skip inlinable objects (do not increment index!)
        // TODO print the root after temp{}, before cycles; could be inlined there
        // HACK m_root is a hack anyway...
        if (ref->m_count == 1 && ref->m_cyclicObjs.empty() && ref != m_root)
        {
            //printf("--skipping inlinable %p\n", ptr);
            continue;
        }

        ref->m_index = index++;

        // Print object
        printf("temp[%d] = ", ref->m_index);
        printObject(ref, 0);
        printf("\n");
    }

    // Iterate over object refs with cycles
    for (auto& parent : ordered)
    {
        for (auto& cycle : parent->m_cyclicObjs)
        {
            // Get the object ref from the pointer
            auto it = m_objectRefs.find(cycle.ptr);
            if (it == m_objectRefs.end())
            {
                printf("warning! temp[%d]%s bad ref\n", parent->m_index, cycle.setter.c_str());
                continue;
            }
            ObjectRef* ref = it->second.get();

            // Use different formatting for setter functions
            if (cycle.setter[0] == ':')
                printf("temp[%d]%s(temp[%d])\n", parent->m_index, cycle.setter.c_str(), ref->m_index);
            else if (cycle.setter[0] == '.')
                printf("temp[%d]%s = temp[%d]\n", parent->m_index, cycle.setter.c_str(), ref->m_index);
            else
                printf("warning! temp[%d]%s expected . or :\n", parent->m_index, cycle.setter.c_str());
        }
    }

    printf("temp = nil\n");
}

void Serializer::printObject(ObjectRef* ref, int indent)
{
    printf("%s", ref->m_constructor.c_str());
    //printf("%*s--debug: refCount = %d, refDepth = %d\n", nested, "", ptr->refCount, ptr->refDepth);
    /*for (auto& attrib : ref->m_immediates)
        printf("\n--debug: %s.%s = %s", attrib.table.c_str(), attrib.key.c_str(), attrib.value.c_str());
    for (auto& attrib : ref->m_directObjs)
        printf("\n--debug: %s.%s = %p", attrib.table.c_str(), attrib.key.c_str(), attrib.ptr);*/

    bool firstLine = true;
    std::string lastTable;

    auto immIt = ref->m_immediates.begin();
    auto immEnd = ref->m_immediates.end();

    auto objIt = ref->m_directObjs.begin();
    auto objEnd = ref->m_directObjs.end();

    // Loop while attributes remain
    while (immIt != immEnd || objIt != objEnd)
    {
        // Alternate between immediates and objects at subtable boundaries
        bool immTurn = immIt != immEnd &&
            (objIt == objEnd || immIt->table.compare(objIt->table) <= 0);

        // Handle change of subtable formatting
        std::string& curTable = immTurn ? immIt->table : objIt->table;
        bool tableChanged = (lastTable.compare(curTable) != 0);

        // Close previous subtable
        if (tableChanged && !lastTable.empty())
        {
            indent -= 2;
            printf("\n%*s}", indent, "");
        }

        // Handle end of line formatting
        if (firstLine)
        {
            printf("\n%*s{\n", indent, "");
            indent += 2;
            firstLine = false;
        }
        else
            printf(",\n");

        // Open new subtable
        if (tableChanged)
        {
            printf("%*s%s =\n%*s{\n", indent, "", curTable.c_str(), indent, "");
            indent += 2;

            lastTable = curTable;
        }

        // Print the next line
        if (immTurn)
        {
            printf("%*s%s = %s", indent, "", immIt->key.c_str(), immIt->value.c_str());

            ++immIt;
        }
        else
        {
            // Get the object ref from the pointer
            auto it = m_objectRefs.find(objIt->ptr);
            if (it == m_objectRefs.end())
            {
                printf("error! temp[%d].%s bad ref\n", ref->m_index, objIt->key.c_str());
                return;
            }

            // Handle nested objects recursively
            if (it->second->m_index == 0)
            {
                printf("%*s%s = ", indent, "", objIt->key.c_str());
                printObject(it->second.get(), indent);
            }
            else
                printf("%*s%s = temp[%d]", indent, "", objIt->key.c_str(), it->second->m_index);

            ++objIt;
        }
    }

    // Add closing braces (newline added by caller)
    if (!firstLine)
    {
        if (!lastTable.empty())
        {
            indent -= 2;
            printf("\n%*s}", indent, "");
        }

        indent -= 2;
        printf("\n%*s}", indent, "");
    }
    else
        printf("{}");
}
