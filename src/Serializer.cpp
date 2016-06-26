#include "Serializer.h"

#include <cassert>

size_t Serializer::addUserdataStore(const void* userdata, const char* name)
{
    size_t index = m_userdataList.size();
    m_userdataList.emplace_back(name);
    m_userdataMap[userdata] = index;
    return index;
}

/*UserdataStore* Serializer::getUserdataStore(const void* userdata)
{
    auto it = m_userdataMap.find(userdata);
    if (it == m_userdataMap.end())
        return nullptr;

    return &m_userdataList[it->second];
}*/

bool Serializer::hasUserdataStore(const void* userdata)
{
    auto it = m_userdataMap.find(userdata);
    return (it != m_userdataMap.end());
}

void Serializer::setAttrib(size_t store, const char* name_, lua_State* L, int index)
{
    std::string name = std::string(name_);
    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        // NOTE this will modify the value on the stack
        lua_pushvalue(L, index);
        m_userdataList[store].immAttribs.emplace_back(name, std::string(lua_tostring(L, -1)));
        lua_pop(L, 1);
        break;
    case LUA_TSTRING:
        {
            std::string str = std::string("\"") + lua_tostring(L, index) + "\"";
            m_userdataList[store].immAttribs.emplace_back(name, str);
            //m_userdataList[store].immAttribs.emplace_back(name, std::string(lua_tostring(L, index)));
        }
        break;
    case LUA_TBOOLEAN:
        m_userdataList[store].immAttribs.emplace_back(name, std::string(lua_toboolean(L, index) ? "true" : "false"));
        break;
    case LUA_TFUNCTION:
        // TODO serialize binary dump? don't forget upvalues
        m_userdataList[store].immAttribs.emplace_back(name, std::string("<function>"));
        break;
    case LUA_TTABLE:
        // TODO we should be able to treat tables effectively the same as userdata
        {
            const void* ptr = serializeTable(L, index);
            m_userdataList[store].mutAttribs.emplace_back(name, ptr);
        }
        break;
    case LUA_TUSERDATA:
        // NOTE mut attribs will have a different form... needs to take setter func name?
        // NOTE should these be pushed to a list in the Serializer rather than in the data store?
        {
            const void* ptr = serializeUserdata(L, index);
            m_userdataList[store].mutAttribs.emplace_back(name, ptr);
        }
        break;
    default:
        // NOTE light userdata, thread?
        printf("can't set attrib %s: unsupported type %s,\n", name_, lua_typename(L, type));
        break;
    }
}

const void* Serializer::serializeUserdata(lua_State* L, int index)
{
    // Get userdata base pointer
    // TODO move topointer up to setAttrib?
    const void* ptr = lua_topointer(L, index);
    assert(ptr != nullptr);

    // Break cycle if this has already been serialized
    if (hasUserdataStore(ptr))
        return ptr;

    // Get userdata serialize function
    luaL_getmetafield(L, index, "serialize");
    assert(lua_type(L, -1) == LUA_TFUNCTION);

    // Push the userdata after the function
    lua_pushvalue(L, (index < 0) ? index - 1 : index); // adjust relative offset
    lua_pushlightuserdata(L, this);

    // Do a protected call; pops function and udata
    // TODO use a regular call instead of a pcall?
    if (lua_pcall(L, 2, 0, 0) != 0)
    {
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
        lua_pop(L, 1); // remove the error string from the stack
        ptr = nullptr;
    }

    return ptr;
}

const void* Serializer::serializeTable(lua_State* L, int index)
{
    // Get table base pointer
    // TODO move topointer up to setAttrib?
    const void* ptr = lua_topointer(L, index);
    assert(ptr != nullptr);

    // Break cycle if this has already been serialized
    if (hasUserdataStore(ptr))
        return ptr;

    // TODO refactor shared code with serializeUserdata?
    // **** Start of code unique to table ****

    size_t store = addUserdataStore(ptr, "");

    lua_pushnil(L);
    while (lua_next(L, (index < 0) ? index - 1 : index))
    {
        int type = lua_type(L, -2);
        switch (type)
        {
        case LUA_TNUMBER:
            {
                lua_pushvalue(L, -2);
                std::string str = std::string("[") + lua_tostring(L, -1) + "]";
                lua_pop(L, 1);
                setAttrib(store, str.c_str(), L, -1);
            }
            break;
        case LUA_TSTRING:
            setAttrib(store, lua_tostring(L, -2), L, -1);
            break;
        default:
            printf("unsupported table key type: %s", lua_typename(L, type));
            break;
        }

        // Remove value, leaving key on top for next iteration of lua_next
        lua_pop(L, 1);
    }

    // **** End of code unique to table *****

    return ptr;
}

void Serializer::print()
{
    const size_t count = m_userdataList.size();
    for (size_t i = 0; i < count; ++i)
    {
        UserdataStore& store = m_userdataList[i];
        printf("store[%ld] = %s\n{\n", i + 1, store.className.c_str());
        for (auto& attrib : store.immAttribs)
        {
            printf("  %s = %s,\n", attrib.first.c_str(), attrib.second.c_str());
        }
        printf("}\n");
    }

    for (size_t i = 0; i < count; ++i)
    {
        UserdataStore& store = m_userdataList[i];
        for (auto& attrib : store.mutAttribs)
        {
            auto it = m_userdataMap.find(attrib.second);
            if (it == m_userdataMap.end())
            {
                printf("warning! store[%ld]:%s(?)\n", i + 1, attrib.first.c_str());
                continue;
            }

            printf("store[%ld]:%s(store[%ld])\n", i + 1, attrib.first.c_str(), it->second + 1);
        }
    }
}
