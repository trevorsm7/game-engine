#pragma once

#include "IUserdata.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>

class TileSet : public TUserdata<TileSet>
{
    std::string m_filename;
    std::vector<uint8_t> m_flags;
    int m_cols, m_rows;

    TileSet(): m_cols(0), m_rows(0) {}

public:
    ~TileSet() {}

    const std::string& getFilename() const {return m_filename;}
    int getCols() const {return m_cols;}
    int getRows() const {return m_rows;}

    static constexpr const uint8_t MoveBlocking = 1;
    static constexpr const uint8_t VisionBlocking = 2;
    bool isFlagSet(int i, uint8_t flag) const {return isValidIndex(i) && m_flags[i-1] & flag;}

    // NOTE using 0 as null tile and 1 as first real tile
    bool isValidIndex(int i) const {return i > 0 && i <= m_flags.size();}
    int getIndexCol(int i) const {return (i-1) % m_cols;}
    int getIndexRow(int i) const {return (i-1) / m_cols;}

private:
    friend class TUserdata<TileSet>;
    void construct(lua_State* L);
    void clone(lua_State* L, TileSet* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_getSize(lua_State* L);

    static constexpr const char* const CLASS_NAME = "TileSet";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"getSize", script_getSize},
        {nullptr, nullptr}
    };
};

class TileMask : public TUserdata<TileMask>
{
    std::vector<uint8_t> m_mask;
    int m_cols, m_rows;

public:
    ~TileMask() {}

    int getCols() const {return m_cols;}
    int getRows() const {return m_rows;}

    // TODO clamp to border instead of failing?

    uint8_t getMask(int i) const {assert(isValidIndex(i)); return m_mask[i];}
    uint8_t getMask(int x, int y) const {assert(isValidIndex(x, y)); return getMask(toIndex(x, y));}

    void setMask(int i, uint8_t val) {assert(isValidIndex(i)); m_mask[i] = val;}
    void setMask(int x, int y, uint8_t val) {assert(isValidIndex(x, y)); setMask(toIndex(x, y), val);}
    void fillMask(uint8_t val) {std::fill(m_mask.begin(), m_mask.end(), val);}

    bool isValidIndex(int i) const {return i >= 0 && i < m_mask.size();}
    bool isValidIndex(int x, int y) const {return x >= 0 && y >= 0 && x < m_cols && y < m_rows;}
    int toIndex(int x, int y) const {return y * m_cols + x;}

private:
    friend class TUserdata<TileMask>;
    void construct(lua_State* L);
    void clone(lua_State* L, TileMask* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_getMask(lua_State* L);
    static int script_fillMask(lua_State* L);
    static int script_fillCircle(lua_State* L);
    static int script_clampMask(lua_State* L);

	// On MSVC, the function pointers don't get instantiated unless we define this first
    template <const uint8_t& (*T)(const uint8_t&, const uint8_t&)>
    static int script_blendMasks(lua_State* L)
	{
		TileMask* tileMaskA = TileMask::checkUserdata(L, 1);
		TileMask* tileMaskB = TileMask::checkUserdata(L, 2);

		const int cols = tileMaskA->m_cols;
		const int rows = tileMaskA->m_rows;

		luaL_argcheck(L, cols == tileMaskB->m_cols, 2, "width doesn't match");
		luaL_argcheck(L, rows == tileMaskB->m_rows, 2, "height doesn't match");

		for (int y = 0; y < rows; ++y)
		{
			for (int x = 0; x < cols; ++x)
			{
				uint8_t maskA = tileMaskA->getMask(x, y);
				uint8_t maskB = tileMaskB->getMask(x, y);
				tileMaskA->setMask(x, y, T(maskA, maskB));
			}
		}

		return 0;
	}

    static constexpr const char* const CLASS_NAME = "TileMask";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"getMask", script_getMask},
        {"fillMask", script_fillMask},
        {"fillCircle", script_fillCircle},
        {"clampMask", script_clampMask},
        {"blendMax", lua_CFunction(script_blendMasks<std::max>)},
        {"blendMin", lua_CFunction(script_blendMasks<std::min>)},
        {nullptr, nullptr}
    };
};

class TileMap : public TUserdata<TileMap>
{
    TileSet* m_tileset;
    TileMask* m_mask;
    std::vector<int> m_map;
    int m_cols, m_rows;

    TileMap(): m_tileset(nullptr), m_mask(nullptr) {}

public:
    std::vector<float> m_debug; // HACK remove
    ~TileMap() {}

    const TileSet* getTileSet() const {return m_tileset;}
    const TileMask* getTileMask() const {return m_mask;}
    int getCols() const {return m_cols;}
    int getRows() const {return m_rows;}

    int getIndex(int i) const {assert(isValidIndex(i)); return m_map[i];}
    int getIndex(int x, int y) const {assert(isValidIndex(x, y)); return getIndex(toIndex(x, y));}

    bool isValidIndex(int i) const {return i >= 0 && i < m_map.size();}
    bool isValidIndex(int x, int y) const {return x >= 0 && y >= 0 && x < m_cols && y < m_rows;}

    bool isFlagSet(int i, uint8_t flag) const {assert(m_tileset); return m_tileset->isFlagSet(getIndex(i), flag);}
    bool isFlagSet(int x, int y, uint8_t flag) const {assert(m_tileset); return m_tileset->isFlagSet(getIndex(x, y), flag);}
    int toIndex(int x, int y) const {return y * m_cols + x;}

private:
    friend class TUserdata<TileMap>;
    void construct(lua_State* L);
    void clone(lua_State* L, TileMap* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_setTileSet(lua_State* L);
    static int script_setTileMask(lua_State* L);
    static int script_getSize(lua_State* L);
    static int script_setSize(lua_State* L);
    static int script_setTiles(lua_State* L);
    static int script_getTile(lua_State* L);
    static int script_moveTiles(lua_State* L);
    static int script_castShadows(lua_State* L);

    static constexpr const char* const CLASS_NAME = "TileMap";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"setTileSet", script_setTileSet},
        {"setTileMask", script_setTileMask},
        {"getSize", script_getSize},
        {"setSize", script_setSize},
        {"setTiles", script_setTiles},
        {"getTile", script_getTile},
        {"moveTiles", script_moveTiles},
        {"castShadows", script_castShadows},
        {nullptr, nullptr}
    };
};
