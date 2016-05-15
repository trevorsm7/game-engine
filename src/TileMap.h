#ifndef __TILEMAP_H__
#define __TILEMAP_H__

#include "TUserdata.h"

#include <memory>
#include <string>
#include <vector>

class TileIndex : public TUserdata<TileIndex>
{
    std::string m_image;
    std::vector<char> m_flags;
    int m_cols, m_rows;

    TileIndex() {}

public:
    ~TileIndex() {}

    const std::string& getImageFile() const {return m_image;}
    bool isCollidable(int i) const {return isValidIndex(i) && (m_flags[i-1] & 1);}
    int getCols() const {return m_cols;}
    int getRows() const {return m_rows;}

    // NOTE using 0 as null tile and 1 as first real tile
    bool isValidIndex(int i) const {return i > 0 && i <= m_cols * m_rows;}
    int getIndexCol(int i) const {return (i-1) % m_cols;}
    int getIndexRow(int i) const {return (i-1) / m_cols;}

    //static TileIndexPtr loadTileIndex(ResourceManager& manager, const std::string& filename);

private:
    friend class TUserdata<TileIndex>;
    void construct(lua_State* L);
    void destroy(lua_State* L) {}

    static constexpr const char* const METATABLE = "TileIndex";
    static int script_getSize(lua_State* L);
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

    TileMap() {}

public:
    ~TileMap() {}

    TileIndex* getTileIndex() {assert(m_index != nullptr); return m_index;}
    int getIndex(int i) const {if (isValidIndex(i)) return m_map[i]; return 0;}
    int getIndex(int x, int y) const {if (isValidIndex(x, y)) return m_map[y * m_cols + x]; return 0;}
    int getCols() const {return m_cols;}
    int getRows() const {return m_rows;}

    bool isValidIndex(int i) const {return i >= 0 && i < m_cols * m_rows;}
    bool isValidIndex(int x, int y) const {return x >= 0 && y >= 0 && x < m_cols && y < m_rows;}

    //static TileMapPtr loadTileMap(ResourceManager& manager, const std::string& filename);

private:
    friend class TUserdata<TileMap>;
    void construct(lua_State* L);
    void destroy(lua_State* L);

    static constexpr const char* const METATABLE = "TileMap";
    static int script_getSize(lua_State* L);
    static constexpr const luaL_Reg METHODS[] =
    {
        {"getSize", script_getSize},
        {nullptr, nullptr}
    };
};

#endif
