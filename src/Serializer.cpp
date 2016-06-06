#include "Serializer.h"

void Serializer::serialize(lua_State* L, int index)
{
    int type = lua_type(L, index);
    switch (type)
    {
    case LUA_TNUMBER:
        // NOTE this will modify the value on the stack
        printf("%s,\n", lua_tostring(L, index));
        break;
    case LUA_TSTRING:
        printf("\"%s\",\n", lua_tostring(L, index));
        break;
    case LUA_TBOOLEAN:
        printf("%s,\n", lua_toboolean(L, index) ? "true" : "false");
        break;
    case LUA_TFUNCTION:
        printf("function<%p>,\n", lua_topointer(L, index));
        break;
    case LUA_TTABLE:
        printf("table<%p>,\n", lua_topointer(L, index));
        break;
    case LUA_TUSERDATA:
        serializeUserdata(L, index);
        printf(",\n");
        break;
    default:
        // NOTE light userdata, thread?
        printf("<unsupported type: %s>,\n", lua_typename(L, type));
        break;
    }
}

void Serializer::serializeUserdata(lua_State* L, int index)
{
    int top = lua_gettop(L);

    if (luaL_getmetafield(L, index, "serialize") == LUA_TFUNCTION)
    {
        // Push the userdata after the function
        lua_pushvalue(L, (index < 0) ? index - 1 : index); // adjust relative offset
        //lua_pushinteger(L, spacing);
        lua_pushlightuserdata(L, this);

        // Do a protected call; pops function and udata
        // TODO use a regular call instead of a pcall?
        if (lua_pcall(L, 2, 0, 0) != 0)
        {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            lua_pop(L, 1); // remove the error string from the stack
        }
    }
    else
    {
        printf("serialize method is unavailable?\n");
    }

    lua_settop(L, top);
}
