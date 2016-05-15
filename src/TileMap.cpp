#include "TileMap.h"
#include "ResourceManager.h"

#include <sstream>
#include <iterator>
#include <algorithm>

void TileIndex::construct(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_pushliteral(L, "sprite");
    luaL_argcheck(L, (lua_rawget(L, 1) == LUA_TSTRING), 1, "{sprite = filename} is required");
    m_image = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "size");
    //if (lua_rawget(L, 1) != LUA_TNIL)
    luaL_argcheck(L, (lua_rawget(L, 1) == LUA_TTABLE), 1, "size required");
    {
        //luaL_checktype(L, -1, LUA_TTABLE);
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        m_cols = luaL_checknumber(L, -2);
        m_rows = luaL_checknumber(L, -1);
        m_flags.resize(m_cols * m_rows);
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "data");
    //if (lua_rawget(L, 1) != LUA_TNIL)
    luaL_argcheck(L, (lua_rawget(L, 1) == LUA_TTABLE), 1, "data required");
    {
        const int size = m_cols * m_rows;
        luaL_argcheck(L, (lua_rawlen(L, -1) == size), 1, "data must match size");
        //luaL_checktype(L, -1, LUA_TTABLE);
        for (int i = 0; i < size; ++i)
        {
            lua_rawgeti(L, -1, i + 1);
            m_flags[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

// NOTE constexpr declaration requires a definition
const luaL_Reg TileIndex::METHODS[];

int TileIndex::script_getSize(lua_State* L)
{
    // Validate function arguments
    TileIndex* index = TileIndex::checkUserdata(L, 1);

    lua_pushnumber(L, index->m_cols);
    lua_pushnumber(L, index->m_rows);

    return 2;
}

void TileMap::construct(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);

    lua_pushliteral(L, "index");
    luaL_argcheck(L, (lua_rawget(L, 1) == LUA_TUSERDATA), 1, "index userdata required");
    m_index = TileIndex::checkUserdata(L, -1);
    m_index->refAdded(L, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "size");
    //if (lua_rawget(L, 1) != LUA_TNIL)
    luaL_argcheck(L, (lua_rawget(L, 1) == LUA_TTABLE), 1, "size required");
    {
        //luaL_checktype(L, -1, LUA_TTABLE);
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        m_cols = luaL_checknumber(L, -2);
        m_rows = luaL_checknumber(L, -1);
        m_map.resize(m_cols * m_rows);
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_pushliteral(L, "data");
    //if (lua_rawget(L, 1) != LUA_TNIL)
    luaL_argcheck(L, (lua_rawget(L, 1) == LUA_TTABLE), 1, "data required");
    {
        const int size = m_cols * m_rows;
        luaL_argcheck(L, (lua_rawlen(L, -1) == size), 1, "data must match size");
        //luaL_checktype(L, -1, LUA_TTABLE);
        for (int i = 0; i < size; ++i)
        {
            lua_rawgeti(L, -1, i + 1);
            m_map[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

void TileMap::destroy(lua_State* L)
{
    if (m_index)
    {
        m_index->refRemoved(L);
        m_index = nullptr;
    }
}

// NOTE constexpr declaration requires a definition
const luaL_Reg TileMap::METHODS[];

int TileMap::script_getSize(lua_State* L)
{
    // Validate function arguments
    TileMap* tilemap = TileMap::checkUserdata(L, 1);

    lua_pushnumber(L, tilemap->m_cols);
    lua_pushnumber(L, tilemap->m_rows);

    return 2;
}

/*TileMapPtr TileMap::loadTileMap(ResourceManager& manager, const std::string& filename)
{
    // Return the resource if it is cached
    IResourcePtr resource = manager.getResource(filename);
    TileMapPtr tilemap = std::dynamic_pointer_cast<TileMap>(resource);
    if (tilemap)
        return tilemap;

    // Load the raw file data into memory
    std::vector<char> data;
    if (!manager.loadRawData(filename, data))
    {
        // TODO: provide a placeholder
        //tilemap = getPlaceholder();
        //manager.bindResource(filename, tilemap);
        return nullptr;
    }

    // Construct empty tilemap
    tilemap = TileMapPtr(new TileMap());

    // Read index file, cols, and rows from raw data
    std::istringstream stream(std::string(data.begin(), data.end()));
    stream >> tilemap->m_index >> tilemap->m_cols >> tilemap->m_rows;

    // Reserve space for the tilemap
    const int cols = tilemap->m_cols;
    const int rows = tilemap->m_rows;
    tilemap->m_map.reserve(cols * rows);

    // Copy the tilemap data from the file
    std::copy(std::istream_iterator<int>(stream), std::istream_iterator<int>(), std::back_inserter(tilemap->m_map));

    // Cache the resource and return it
    manager.bindResource(filename, tilemap);
    return tilemap;
}*/

/*TileIndexPtr TileIndex::loadTileIndex(ResourceManager& manager, const std::string& filename)
{
    // Return the resource if it is cached
    IResourcePtr resource = manager.getResource(filename);
    TileIndexPtr tileindex = std::dynamic_pointer_cast<TileIndex>(resource);
    if (tileindex)
        return tileindex;

    // Load the raw file data into memory
    std::vector<char> data;
    if (!manager.loadRawData(filename, data))
    {
        // TODO: provide a placeholder
        //tileindex = getPlaceholder();
        //manager.bindResource(filename, tileindex);
        return nullptr;
    }

    // Construct empty tile index
    tileindex = TileIndexPtr(new TileIndex());

    // Read image file, cols, and rows from raw data
    std::istringstream stream(std::string(data.begin(), data.end()));
    stream >> tileindex->m_image >> tileindex->m_cols >> tileindex->m_rows;

    // Reserve space for the tile flags
    const int cols = tileindex->m_cols;
    const int rows = tileindex->m_rows;
    tileindex->m_flags.reserve(cols * rows);

    // Copy the tile flag data from the file
    std::copy(std::istream_iterator<int>(stream), std::istream_iterator<int>(), std::back_inserter(tileindex->m_flags));

    // Cache the resource and return it
    manager.bindResource(filename, tileindex);
    return tileindex;
}*/
