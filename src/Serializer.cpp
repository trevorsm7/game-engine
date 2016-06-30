#include "Serializer.h"

#include <list>
#include <algorithm>
#include <cassert>

UserdataStore* Serializer::addUserdataStore(const void* userdata, int depth)
{
    assert(getUserdataStore(userdata) == nullptr);
    UserdataStore* ptr = new UserdataStore(depth);
    //m_userdataMap.emplace(userdata, ptr);
    m_userdataMap[userdata] = UserdataStorePtr(ptr);
    return ptr;
}

UserdataStore* Serializer::getUserdataStore(const void* userdata)
{
    auto it = m_userdataMap.find(userdata);
    if (it == m_userdataMap.end())
        return nullptr;

    return it->second.get();
}

void Serializer::setAttrib(UserdataStore* store, const char* name, const char* setter, lua_State* L, int index)
{
    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        // NOTE this will modify the value on the stack
        lua_pushvalue(L, index);
        store->immAttribs.emplace_back(std::string(name), std::string(lua_tostring(L, -1)));
        lua_pop(L, 1);
        break;
    case LUA_TSTRING:
        {
            std::string str = std::string("\"") + lua_tostring(L, index) + "\"";
            store->immAttribs.emplace_back(std::string(name), str);
        }
        break;
    case LUA_TBOOLEAN:
        store->immAttribs.emplace_back(std::string(name), std::string(lua_toboolean(L, index) ? "true" : "false"));
        break;
    case LUA_TFUNCTION:
        // TODO serialize binary dump? don't forget upvalues
        store->immAttribs.emplace_back(std::string(name), std::string("<function>"));
        break;
    case LUA_TTABLE:
    case LUA_TUSERDATA:
        serializeMutable(store, name, setter, L, index);
        break;
    default:
        // NOTE light userdata, thread?
        printf("can't set attrib %s: unsupported type %s,\n", name, lua_typename(L, type));
        break;
    }
}

void Serializer::serializeMutable(UserdataStore* parent, const char* name, const char* setter, lua_State* L, int index)
{
    // Get userdata pointer and data store
    const void* ptr = lua_topointer(L, index);
    assert(ptr != nullptr);
    UserdataStore* store = getUserdataStore(ptr);

    // Check if data store already exists
    if (store)
    {
        store->refCount++;

        // Break cycle if this has already been serialized
        if (store->onStack)
        {
            if (parent)
            {
                // TODO use a flag instead?
                std::string key = std::string(1, ':') + setter;
                if (key.length() == 1)
                    key = std::string(1, '.') + name;
                parent->cycleAttribs.emplace_back(key, ptr);
            }
            else // HACK
                m_root = store;

            return;
        }
    }
    else
    {
        // Create the data store
        const int depth = parent ? parent->refDepth + 1 : 0;
        store = addUserdataStore(ptr, depth);

        const int type = lua_type(L, index);
        assert(type == LUA_TUSERDATA || type == LUA_TTABLE);

        // Handle userdata and tables separately
        if (type == LUA_TUSERDATA)
        {
            // Get userdata serialize function
            luaL_getmetafield(L, index, "serialize");
            assert(lua_type(L, -1) == LUA_TFUNCTION);

            // Push the userdata after the function
            lua_pushvalue(L, (index < 0) ? index - 1 : index); // adjust relative offset
            lua_pushlightuserdata(L, this);

            // Do a regular call; pops function and udata
            lua_call(L, 2, 0);
        }
        else
        {
            // Iterate over table key/value pairs
            // TODO it might be too overflow the Lua stack if we have a chain of tables; break this up with lua_call as with userdata?
            lua_pushnil(L);
            const int adjIndex = (index < 0) ? index - 1 : index;
            while (lua_next(L, adjIndex))
            {
                // Handle different key types
                int type = lua_type(L, -2);
                switch (type)
                {
                case LUA_TNUMBER:
                    {
                        lua_pushvalue(L, -2);
                        std::string str = std::string("[") + lua_tostring(L, -1) + "]";
                        lua_pop(L, 1);
                        setAttrib(store, str.c_str(), "", L, -1);
                    }
                    break;
                case LUA_TSTRING:
                    setAttrib(store, lua_tostring(L, -2), "", L, -1);
                    break;
                case LUA_TBOOLEAN:
                    {
                        const char* key = lua_toboolean(L, -2) ? "[true]" : "[false]";
                        setAttrib(store, key, "", L, -1);
                    }
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
    }

    // Clear the onStack flag
    assert(store != nullptr);
    store->onStack = false;

    // Propagate depth balancing down to parent
    if (parent)
    {
        if (parent->refDepth >= store->refDepth)
            parent->refDepth = store->refDepth - 1;

        parent->mutAttribs.emplace_back(std::string(name), ptr);
    }
    else // HACK
        m_root = store;
}

void Serializer::printObject(UserdataStore* ptr, int indent)
{
    const int nested = indent + 2;
    printf("%s\n%*s{\n", ptr->className.c_str(), indent, "");
    //printf("%*s--debug: refCount = %d, refDepth = %d\n", nested, "", ptr->refCount, ptr->refDepth);

    bool firstLine = true;

    // Print immutable attributes
    for (auto& attrib : ptr->immAttribs)
    {
        if (!firstLine)
            printf(",\n");
        firstLine = false;

        printf("%*s%s = %s", nested, "", attrib.first.c_str(), attrib.second.c_str());
    }

    // Iterate over mutable refs without cycles
    for (auto& attrib : ptr->mutAttribs)
    {
        if (!firstLine)
            printf(",\n");
        firstLine = false;

        // Get the data store from the pointer
        auto it = m_userdataMap.find(attrib.second);
        if (it == m_userdataMap.end())
        {
            printf("warning! store[%d].%s bad ref\n", ptr->index, attrib.first.c_str());
            continue;
        }

        // Handle nested objects recursively
        if (it->second->index == 0)
        {
            printf("%*s%s = ", nested, "", attrib.first.c_str());
            printObject(it->second.get(), nested);
            continue;
        }

        printf("%*s%s = store[%d]", nested, "", attrib.first.c_str(), it->second->index);
    }

    // NOTE newline added by caller
    // TODO collapse to {} if no attributes set
    printf("\n%*s}", indent, "");
}

void Serializer::print()
{
    // TODO clean this up!
    // TODO write to buffer/file instead of printf

    std::list<UserdataStore*> ordered;

    // Sort data stores into list by ref depth, descending order
    for (auto& pair : m_userdataMap)
    {
        // Reset index before ordering
        pair.second->index = 0;

        // Find first index with lower ref depth
        int depth = pair.second->refDepth;
        auto it2 = std::find_if(ordered.begin(), ordered.end(),
            [depth](UserdataStore* ptr) {return ptr->refDepth < depth;});

        // Insert before index with lower ref depth
        ordered.insert(it2, pair.second.get());
    }

    printf("store = {}\n");

    int index = 1;
    for (auto& ptr : ordered)
    {
        // Skip inlinable objects (do not increment index!)
        // TODO print the root after store{}, before cycles; could be inlined there
        // HACK m_root is a hack anyway...
        if (ptr->refCount == 1 && ptr->cycleAttribs.empty() && ptr != m_root)
        {
            //printf("--skipping inlinable %p\n", ptr);
            continue;
        }

        ptr->index = index++;

        // Print object
        printf("store[%d] = ", ptr->index);
        printObject(ptr, 0);
        printf("\n");
    }

    // Iterate over mutable refs with cycles
    for (auto& ptr : ordered)
    {
        for (auto& attrib : ptr->cycleAttribs)
        {
            // Get the data store from the pointer
            auto it = m_userdataMap.find(attrib.second);
            if (it == m_userdataMap.end())
            {
                printf("warning! store[%d]%s bad ref\n", ptr->index, attrib.first.c_str());
                continue;
            }

            // Use different formatting for setter functions
            if (attrib.first[0] == ':')
                printf("store[%d]%s(store[%d])\n", ptr->index, attrib.first.c_str(), it->second->index);
            else if (attrib.first[0] == '.')
                printf("store[%d]%s = store[%d]\n", ptr->index, attrib.first.c_str(), it->second->index);
            else
                printf("warning! store[%d]%s expected . or :\n", ptr->index, attrib.first.c_str());
        }
    }

    printf("store = nil\n");
}
