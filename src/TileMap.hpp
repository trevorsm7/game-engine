#pragma once

#include "IUserdata.hpp"

#include <string>
#include <vector>
#include <cstdint>

class TileIndex : public TUserdata<TileIndex>
{
    std::string m_image;
    std::vector<uint8_t> m_flags;
    int m_cols, m_rows;

    TileIndex(): m_cols(0), m_rows(0) {}

public:
    ~TileIndex() {}

    const std::string& getImageFile() const {return m_image;}
    int getCols() const {return m_cols;}
    int getRows() const {return m_rows;}

    bool isCollidable(int i) const {assert(isValidIndex(i)); return i > 0 && m_flags[i-1] & 1;}

    // NOTE using 0 as null tile and 1 as first real tile
    bool isValidIndex(int i) const {return i >= 0 && i <= m_cols * m_rows;}
    int getIndexCol(int i) const {return (i-1) % m_cols;}
    int getIndexRow(int i) const {return (i-1) / m_cols;}

private:
    friend class TUserdata<TileIndex>;
    void construct(lua_State* L);
    void clone(lua_State* L, TileIndex* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_getSize(lua_State* L);

    static constexpr const char* const CLASS_NAME = "TileIndex";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"getSize", script_getSize},
        {nullptr, nullptr}
    };
};

class TileMap : public TUserdata<TileMap>
{
    TileIndex* m_index;
    std::vector<int> m_map;
    int m_cols, m_rows;

    TileMap(): m_index(nullptr) {}

public:
    ~TileMap() {}

    const TileIndex* getTileIndex() const {return m_index;}
    int getIndex(int i) const {assert(isValidIndex(i)); return m_map[i];}
    int getIndex(int x, int y) const {assert(isValidIndex(x, y)); return m_map[toIndex(x, y)];}
    int getCols() const {return m_cols;}
    int getRows() const {return m_rows;}

    bool isValidIndex(int i) const {return i >= 0 && i < m_cols * m_rows;}
    bool isValidIndex(int x, int y) const {return x >= 0 && y >= 0 && x < m_cols && y < m_rows;}

    bool isCollidable(int i) const {return isCollidableIndex(getIndex(i));}
    bool isCollidable(int x, int y) const {return isCollidableIndex(getIndex(x, y));}

private:
    int toIndex(int x, int y) const {return y * m_cols + x;}

    bool isCollidableIndex(int i) const {assert(m_index); return m_index->isValidIndex(i) && m_index->isCollidable(i);}

private:
    friend class TUserdata<TileMap>;
    void construct(lua_State* L);
    void clone(lua_State* L, TileMap* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_setTileIndex(lua_State* L);
    static int script_getSize(lua_State* L);
    static int script_setSize(lua_State* L);
    static int script_setTiles(lua_State* L);
    static int script_getTile(lua_State* L);
    static int script_moveTiles(lua_State* L);

    static constexpr const char* const CLASS_NAME = "TileMap";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"setTileIndex", script_setTileIndex},
        {"getSize", script_getSize},
        {"setSize", script_setSize},
        {"setTiles", script_setTiles},
        {"getTile", script_getTile},
        {"moveTiles", script_moveTiles},
        {nullptr, nullptr}
    };
};
