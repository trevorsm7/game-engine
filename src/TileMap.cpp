#include "TileMap.hpp"
#include "Serializer.hpp"
#include "ResourceManager.hpp"

//#include <iterator>
#include <algorithm>

const luaL_Reg TileIndex::METHODS[];
const luaL_Reg TileMap::METHODS[];

// =============================================================================
// TileIndex
// =============================================================================

void TileIndex::construct(lua_State* L)
{
    lua_pushliteral(L, "sprite");
    luaL_argcheck(L, (lua_rawget(L, 2) == LUA_TSTRING), 2, "{sprite = filename} is required");
    m_image = lua_tostring(L, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "size");
    luaL_argcheck(L, (lua_rawget(L, 2) == LUA_TTABLE), 2, "size required");
    lua_rawgeti(L, -1, 1);
    lua_rawgeti(L, -2, 2);
    m_cols = luaL_checknumber(L, -2);
    m_rows = luaL_checknumber(L, -1);
    m_flags.resize(m_cols * m_rows);
    lua_pop(L, 3);

    lua_pushliteral(L, "data");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        const int size = m_cols * m_rows;
        luaL_checktype(L, -1, LUA_TTABLE);
        luaL_argcheck(L, (lua_rawlen(L, -1) == size), 2, "data must match size");
        for (int i = 0; i < size; ++i)
        {
            lua_rawgeti(L, -1, i + 1);
            m_flags[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

void TileIndex::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    ref->setString("", "sprite", m_image);
    int size[] = {m_cols, m_rows};
    ref->setArray("", "size", size, 2);
    ref->setArray("", "data", m_flags.data(), m_cols * m_rows);
}

int TileIndex::script_getSize(lua_State* L)
{
    // Validate function arguments
    TileIndex* index = TileIndex::checkUserdata(L, 1);

    lua_pushnumber(L, index->m_cols);
    lua_pushnumber(L, index->m_rows);
    return 2;
}

// =============================================================================
// TileMap
// =============================================================================

// TODO refactor with similar functions in Actor? move template to TUserdata?
void TileMap::setTileIndex(lua_State* L, int index)
{
    TileIndex* tileIndex = TileIndex::checkUserdata(L, index);

    // Do nothing if we already own the component
    if (m_index == tileIndex)
        return;

    // Clear old component first
    if (m_index != nullptr)
        m_index->refRemoved(L);

    // Add component to new actor
    tileIndex->refAdded(L, index);
    m_index = tileIndex;
}

void TileMap::construct(lua_State* L)
{
    lua_pushliteral(L, "index");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setTileIndex(L, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "size");
    luaL_argcheck(L, (lua_rawget(L, 2) == LUA_TTABLE), 2, "size required");
    lua_rawgeti(L, -1, 1);
    lua_rawgeti(L, -2, 2);
    m_cols = luaL_checknumber(L, -2);
    m_rows = luaL_checknumber(L, -1);
    m_map.resize(m_cols * m_rows);
    lua_pop(L, 3);

    lua_pushliteral(L, "data");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        const int size = m_cols * m_rows;
        luaL_checktype(L, -1, LUA_TTABLE);
        luaL_argcheck(L, (lua_rawlen(L, -1) == size), 1, "data must match size");
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

void TileMap::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    if (m_index)
    {
        m_index->pushUserdata(L);
        serializer->serializeMember(ref, "", "index", "setTileIndex", L, -1);
        lua_pop(L, 1);
    }

    int size[] = {m_cols, m_rows};
    ref->setArray("", "size", size, 2);
    ref->setArray("", "data", m_map.data(), m_cols * m_rows);
}

int TileMap::script_setTileIndex(lua_State* L)
{
    // Validate function arguments
    TileMap* tilemap = TileMap::checkUserdata(L, 1);

    tilemap->setTileIndex(L, 2);

    return 0;
}

int TileMap::script_getSize(lua_State* L)
{
    // Validate function arguments
    TileMap* tilemap = TileMap::checkUserdata(L, 1);

    lua_pushnumber(L, tilemap->m_cols);
    lua_pushnumber(L, tilemap->m_rows);
    return 2;
}

int TileMap::script_setSize(lua_State* L)
{
    // Validate function arguments
    TileMap* const tilemap = TileMap::checkUserdata(L, 1);
    const int w = luaL_checkinteger(L, 2);
    const int h = luaL_checkinteger(L, 3);

    const int cols = tilemap->m_cols;
    const int rows = tilemap->m_rows;

    luaL_argcheck(L, (w > 0), 4, "w must be greater than 0");
    luaL_argcheck(L, (h > 0), 5, "h must be greater than 0");

    tilemap->m_cols = w;
    tilemap->m_rows = h;

    // Return early if w hasn't changed; can simply resize
    if (w == cols)
    {
        tilemap->m_map.resize(w * h, 0);

        // Return self userdata
        lua_pushvalue(L, 1);
        return 1;
    }

    // TODO Do we need to preserve the old data? Could just resize and clear

    // For resizing purposes, the last row is the lesser of the old and the new
    const auto i = tilemap->m_map.begin();
    const int lastRow = std::min(rows, h) - 1;

    if (w > cols)
    {
        // Resize first so we can shift the old rows into the new space
        tilemap->m_map.resize(w * h, 0);

        // Shift the old rows outward starting from the end
        for (int row = lastRow; row > 0; --row)
        {
            const int old_i = row * cols;
            const int new_i = row * w;
            std::move_backward(i + old_i, i + (old_i + cols), i + (new_i + cols));
            std::fill(i + (new_i - (w - cols)), i + new_i, 0);
        }
    }
    else //if (w < cols)
    {
        // Shift the old rows inward starting from the beginning
        for (int row = 1; row <= lastRow; ++row)
        {
            const int old_i = row * cols;
            const int new_i = row * w;
            std::move(i + old_i,  i + (old_i + w), i + new_i);
        }

        // Resize after we shifting the old rows out of the old space
        tilemap->m_map.resize(w * h, 0);
    }

    // Fill cells from the old last row up to the start of the resize
    const int newEnd = lastRow * w + std::min(cols, w);
    const int oldEnd = std::min(rows * cols, w * h);
    if (newEnd < oldEnd)
        std::fill(i + newEnd, i + oldEnd, 0);

    return 0;
}

int TileMap::script_setTiles(lua_State* L)
{
    // Validate function arguments
    TileMap* const tilemap = TileMap::checkUserdata(L, 1);
    const int x = luaL_checkinteger(L, 2);
    const int y = luaL_checkinteger(L, 3);
    const int w = luaL_checkinteger(L, 4);
    const int h = luaL_checkinteger(L, 5);
    const int val = luaL_checkinteger(L, 6);

    const int cols = tilemap->m_cols;
    const int rows = tilemap->m_rows;

    luaL_argcheck(L, (x >= 0 && x < cols), 2, "x is out of bounds");
    luaL_argcheck(L, (y >= 0 && y < rows), 3, "y is out of bounds");
    luaL_argcheck(L, (w >= 0), 4, "w must be positive");
    luaL_argcheck(L, (h >= 0), 5, "h must be positive");
    luaL_argcheck(L, (x + w <= cols), 4, "x + w is out of bounds");
    luaL_argcheck(L, (y + h <= rows), 5, "y + h is out of bounds");

    if (w == 0 || h == 0)
        return 0;

    int index = tilemap->toIndex(x, y);
    for (int row = 0; row < h; ++row)
    {
        /*const int end = index + w;
        for (int i = index; i < end; ++i)
            tilemap->m_map[i] = val;*/
        auto i = tilemap->m_map.begin() + index;
        std::fill_n(i, w, val);
        index += cols;
    }

    return 0;
}

int TileMap::script_getTile(lua_State* L)
{
    // Validate function arguments
    TileMap* tilemap = TileMap::checkUserdata(L, 1);
    const int x = luaL_checkinteger(L, 2);
    const int y = luaL_checkinteger(L, 3);

    const int cols = tilemap->m_cols;
    const int rows = tilemap->m_rows;

    luaL_argcheck(L, (x >= 0 && x < cols), 2, "x is out of bounds");
    luaL_argcheck(L, (y >= 0 && y < rows), 3, "y is out of bounds");

    lua_pushinteger(L, tilemap->getIndex(x, y));
    return 1;
}

int TileMap::script_moveTiles(lua_State* L)
{
    // Validate function arguments
    TileMap* const tilemap = TileMap::checkUserdata(L, 1);
    const int x = luaL_checkinteger(L, 2);
    const int y = luaL_checkinteger(L, 3);
    const int w = luaL_checkinteger(L, 4);
    const int h = luaL_checkinteger(L, 5);
    const int dx = luaL_checkinteger(L, 6);
    const int dy = luaL_checkinteger(L, 7);

    const int cols = tilemap->m_cols;
    const int rows = tilemap->m_rows;

    luaL_argcheck(L, (x >= 0 && x < cols), 2, "x is out of bounds");
    luaL_argcheck(L, (y >= 0 && y < rows), 3, "y is out of bounds");
    luaL_argcheck(L, (w >= 0), 4, "w must be positive");
    luaL_argcheck(L, (h >= 0), 5, "h must be positive");
    luaL_argcheck(L, (x + w <= cols), 4, "x + w is out of bounds");
    luaL_argcheck(L, (y + h <= rows), 5, "y + h is out of bounds");
    luaL_argcheck(L, (x + dx >= 0 && x + w + dx <= cols), 6, "x + dx is out of bounds");
    luaL_argcheck(L, (y + dy >= 0 && y + h + dy <= rows), 6, "y + dy is out of bounds");

    if (w == 0 || h == 0)
        return 0;

    const int old_i = tilemap->toIndex(x, y);
    const int new_i = tilemap->toIndex(x + dx, y + dy);

    if (new_i > old_i)
    {
        for (int row = h-1; row >= 0; --row)
        {
            auto i = tilemap->m_map.begin() + (row * cols);
            std::move_backward(i + old_i, i + (old_i + w), i + (new_i + w));
        }
    }
    else if (new_i < old_i)
    {
        for (int row = 0; row < h; ++row)
        {
            auto i = tilemap->m_map.begin() + (row * cols);
            std::move(i + old_i,  i + (old_i + w), i + new_i);
        }
    }

    return 0;
}
