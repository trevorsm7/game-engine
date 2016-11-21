#include "TileMap.hpp"
#include "Serializer.hpp"
#include "ResourceManager.hpp"

#include <cstdlib>
#include <chrono>

const luaL_Reg TileIndex::METHODS[];
const luaL_Reg TileMask::METHODS[];
const luaL_Reg TileMap::METHODS[];

template <class T>
static void constructVector(lua_State* L, int& cols, int& rows, std::vector<T>& vec)
{
    lua_pushliteral(L, "size");
    luaL_argcheck(L, (lua_rawget(L, 2) == LUA_TTABLE), 2, "size required");
    lua_rawgeti(L, -1, 1);
    lua_rawgeti(L, -2, 2);
    cols = luaL_checkinteger(L, -2);
    rows = luaL_checkinteger(L, -1);
    lua_pop(L, 3);

    const int size = cols * rows;
    vec.resize(size);

    lua_pushliteral(L, "data");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        luaL_checktype(L, -1, LUA_TTABLE);
        luaL_argcheck(L, (lua_rawlen(L, -1) == size), 2, "data must match size");
        for (int i = 0; i < size; ++i)
        {
            lua_rawgeti(L, -1, i + 1);
            vec[i] = luaL_checkinteger(L, -1); // NOTE need to template if we have non-integer data
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

// =============================================================================
// TileIndex
// =============================================================================

void TileIndex::construct(lua_State* L)
{
    lua_pushliteral(L, "sprite");
    luaL_argcheck(L, (lua_rawget(L, 2) == LUA_TSTRING), 2, "{sprite = filename} is required");
    m_image = lua_tostring(L, -1);
    lua_pop(L, 1);

    constructVector(L, m_cols, m_rows, m_flags);
}

void TileIndex::clone(lua_State* L, TileIndex* source)
{
    m_image = source->m_image;
    m_flags = source->m_flags;
    m_cols = source->m_cols;
    m_rows = source->m_rows;
}

void TileIndex::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    serializer->setString(ref, "", "sprite", m_image);
    int size[] = {m_cols, m_rows};
    serializer->setArray(ref, "", "size", size, 2);
    serializer->setArray(ref, "", "data", m_flags);
}

int TileIndex::script_getSize(lua_State* L)
{
    // Validate function arguments
    TileIndex* index = TileIndex::checkUserdata(L, 1);

    lua_pushinteger(L, index->m_cols);
    lua_pushinteger(L, index->m_rows);
    return 2;
}

// =============================================================================
// TileMask
// =============================================================================

void TileMask::construct(lua_State* L)
{
    constructVector(L, m_cols, m_rows, m_mask);
}

void TileMask::clone(lua_State* L, TileMask* source)
{
    m_mask = source->m_mask;
    m_cols = source->m_cols;
    m_rows = source->m_rows;
}

void TileMask::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    int size[] = {m_cols, m_rows};
    serializer->setArray(ref, "", "size", size, 2);
    serializer->setArray(ref, "", "data", m_mask);
}

int TileMask::script_getMask(lua_State* L)
{
    TileMask* tileMask = TileMask::checkUserdata(L, 1);
    const int x = luaL_checkinteger(L, 2);
    const int y = luaL_checkinteger(L, 3);

    luaL_argcheck(L, (x >= 0 && x < tileMask->m_cols), 2, "x is out of bounds");
    luaL_argcheck(L, (y >= 0 && y < tileMask->m_rows), 3, "y is out of bounds");

    lua_pushinteger(L, tileMask->getMask(x, y));
    return 1;
}

int TileMask::script_fillMask(lua_State* L)
{
    TileMask* tileMask = TileMask::checkUserdata(L, 1);
    const int val = luaL_optinteger(L, 2, 0);

    tileMask->fillMask(val);

    return 0;
}

int TileMask::script_fillCircle(lua_State* L)
{
    TileMask* tileMask = TileMask::checkUserdata(L, 1);
    const int x = luaL_checkinteger(L, 2);
    const int y = luaL_checkinteger(L, 3);
    const int radius = luaL_checkinteger(L, 4);
    const int inside = luaL_optinteger(L, 5, 255);
    const int outside = luaL_optinteger(L, 6, 0);

    const int rows = tileMask->m_rows;
    const int cols = tileMask->m_cols;

    //luaL_argcheck(L, (x >= 0 && x < cols), 2, "x is out of bounds");
    //luaL_argcheck(L, (y >= 0 && y < rows), 3, "y is out of bounds");
    luaL_argcheck(L, (radius >= 0), 4, "radius must be positive");

    const int ylb = std::max(y - radius, 0);
    const int yhb = std::min(y + radius, rows - 1);
    const int xlb = std::max(x - radius, 0);
    const int xhb = std::min(x + radius, cols - 1);

    // Fill the outside of the circle
    tileMask->fillMask(outside);

    // Fill the inside of the circle
    const int r2 = radius * radius;
    for (int yi = ylb; yi <= yhb; ++yi)
    {
        const int y2 = (yi - y) * (yi - y);
        for (int xi = xlb; xi <= xhb; ++xi)
        {
            const int x2 = (xi - x) * (xi - x);
            if (x2 + y2 <= r2)
                tileMask->setMask(xi, yi, inside);
        }
    }

    return 0;
}

int TileMask::script_clampMask(lua_State* L)
{
    TileMask* tileMask = TileMask::checkUserdata(L, 1);
    const int low = luaL_checkinteger(L, 2);
    const int high = luaL_checkinteger(L, 3);

    for (int y = 0; y < tileMask->m_rows; ++y)
    {
        for (int x = 0; x < tileMask->m_cols; ++x)
        {
            int mask = tileMask->getMask(x, y);
            //mask = std::clamp(mask, low, high); // C++17
            mask = std::min(std::max(mask, low), high);
            tileMask->setMask(x, y, mask);
        }
    }

    return 0;
}

template <const uint8_t& (*T)(const uint8_t&, const uint8_t&)>
int TileMask::script_blendMasks(lua_State* L)
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

// =============================================================================
// TileMap
// =============================================================================

void TileMap::construct(lua_State* L)
{
    lua_pushliteral(L, "index");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setChild(L, m_index, -1);
    lua_pop(L, 1);

    lua_pushliteral(L, "mask");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setChild(L, m_mask, -1);
    lua_pop(L, 1);

    constructVector(L, m_cols, m_rows, m_map);
}

void TileMap::clone(lua_State* L, TileMap* source)
{
    if (source->m_index)
    {
        source->m_index->pushUserdata(L); // shallow copy
        setChild(L, m_index, -1);
        lua_pop(L, 1);
    }

    if (source->m_mask)
    {
        source->m_mask->pushUserdata(L); // shallow copy
        setChild(L, m_mask, -1);
        lua_pop(L, 1);
    }

    m_map = source->m_map;
    m_cols = source->m_cols;
    m_rows = source->m_rows;
}

void TileMap::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    serializer->serializeMember(ref, "", "index", "setTileIndex", L, m_index);
    serializer->serializeMember(ref, "", "mask", "setTileMask", L, m_mask);

    int size[] = {m_cols, m_rows};
    serializer->setArray(ref, "", "size", size, 2);
    serializer->setArray(ref, "", "data", m_map);
}

int TileMap::script_setTileIndex(lua_State* L)
{
    TileMap* tilemap = TileMap::checkUserdata(L, 1);
    tilemap->setChild(L, tilemap->m_index, 2);
    return 0;
}

int TileMap::script_setTileMask(lua_State* L)
{
    TileMap* tilemap = TileMap::checkUserdata(L, 1);
    tilemap->setChild(L, tilemap->m_mask, 2);
    return 0;
}

int TileMap::script_getSize(lua_State* L)
{
    // Validate function arguments
    TileMap* tilemap = TileMap::checkUserdata(L, 1);

    lua_pushinteger(L, tilemap->m_cols);
    lua_pushinteger(L, tilemap->m_rows);
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
        return 0;
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

class ShadowVector
{
    std::vector<std::pair<float, float>> m_vec;
    int m_mode;

    static constexpr const float k_edge = 0.5f;
    static constexpr const float k_corner = 0.5f;
    static constexpr const float k_radius = 0.5f;

public:
    ShadowVector(int mode = 1): m_mode(mode)
    {
        // TODO add parameter for slopes/angle
        m_vec.emplace_back(-1.f, 1.f); // visibility in 90 degree cone
    }

    void offsetToSlopes(int depth, int offset, float& slopeLow, float& slopeHigh)
    {
        if (m_mode == 1)
        {
            slopeLow = float(offset - k_edge) / float(depth);
            slopeHigh = float(offset + k_edge) / float(depth);
        }
        else if (m_mode == 2)
        {
            const float radius2 = k_radius * k_radius;
            const float length2 = depth * depth + offset * offset;
            const float factor = std::sqrtf(length2 / radius2 - 1.f);
            slopeLow = (offset * factor - depth) / (depth * factor + offset);
            slopeHigh = (offset * factor + depth) / (depth * factor - offset);
        }
        else if (m_mode == 3)
        {
            slopeLow = std::min(
                float(offset - k_edge) / float(depth - k_corner),
                float(offset - k_edge) / float(depth + k_corner));
            slopeHigh = std::max(
                float(offset + k_edge) / float(depth - k_corner),
                float(offset + k_edge) / float(depth + k_corner));
        }
    }

    void slopesToOffsets(int depth, float slopeLow, float slopeHigh, int& offsetLow, int& offsetHigh)
    {
        if (m_mode == 1)
        {
            offsetLow = std::floor(slopeLow * depth + k_edge);
            offsetHigh = std::ceil(slopeHigh * depth - k_edge);
        }
        else if (m_mode == 2)
        {
            const float stepLow = k_radius * std::sqrtf(1.f + (slopeLow * slopeLow));
            const float stepHigh = k_radius * std::sqrtf(1.f + (slopeHigh * slopeHigh));
            offsetLow = std::floor(slopeLow * depth + stepLow);
            offsetHigh = std::ceil(slopeHigh * depth - stepHigh);
        }
        else if (m_mode == 3)
        {
            offsetLow = std::floor(std::min(
                slopeLow * (depth - k_corner),
                slopeLow * (depth + k_corner)) + k_edge);
            offsetHigh = std::ceil(std::max(
                slopeHigh * (depth - k_corner),
                slopeHigh * (depth + k_corner)) - k_edge);
        }
    }

    void getVisible(int depth, int limitLow, int limitHigh, std::vector<int>& visible)
    {
        visible.clear();

        int offset = limitLow;
        for (auto& shadow : m_vec)
        {
            int offsetLow, offsetHigh;
            slopesToOffsets(depth, shadow.first, shadow.second, offsetLow, offsetHigh);
            offsetHigh = std::min(offsetHigh, limitHigh);
            offset = std::max(offset, offsetLow);

            if (offset > limitHigh)
                break;

            // Count from start to end of visible region
            for (; offset <= offsetHigh; ++offset)
                visible.push_back(offset);
        }
    }

    void castShadow(int depth, int offset)
    {
        if (m_vec.empty())
            return;

        // Cast shadow at the midpoint of the block
        assert(depth >= 1);
        float slope1, slope2;
        offsetToSlopes(depth, offset, slope1, slope2);
        assert(slope2 > slope1);

        for (auto it = m_vec.begin(), end = m_vec.end(); it < end; ++it)
        {
            // Already shadowed
            if (slope2 <= it->first)
                return;

            // Keep looking
            if (slope1 >= it->second)
                continue;

            // Completely cover visible region
            if (slope1 <= it->first && slope2 >= it->second)
            {
                it = m_vec.erase(it);
                end = m_vec.end();
                continue;
            }

            // Split visible region
            if (slope1 > it->first && slope2 < it->second)
            {
                float temp = it->second;
                it->second = slope1;
                it = m_vec.emplace(it+1, slope2, temp);
                //end = m_vec.end();
                return;
            }

            // Partial overlap with visible region
            if (slope1 <= it->first)
            {
                it->first = slope2;
                return;
            }

            // Partial overlap with visible region
            if (slope2 >= it->second)
            {
                it->second = slope1;
                continue;
            }

            assert(false);
        }
    }

    void debug(int x, int y, int dx, int dy, int ii, std::vector<float>& lines)
    {
        float i = ii - 0.75f;
        auto end = m_vec.end();
        for (auto it = m_vec.begin(); it < end; ++it)
        {
            lines.push_back(4);
            lines.push_back(x + 0.5f + dx * i + abs(dy) * (it->first * i));
            lines.push_back(y + 0.5f + abs(dx) * (it->first * i) + dy * i);
            lines.push_back(x + 0.5f + dx * (i+1) + abs(dy) * (it->first * (i+1)));
            lines.push_back(y + 0.5f + abs(dx) * (it->first * (i+1)) + dy * (i+1));
            lines.push_back(x + 0.5f + dx * (i+1) + abs(dy) * (it->second * (i+1)));
            lines.push_back(y + 0.5f + abs(dx) * (it->second * (i+1)) + dy * (i+1));
            lines.push_back(x + 0.5f + dx * i + abs(dy) * (it->second * i));
            lines.push_back(y + 0.5f + abs(dx) * (it->second * i) + dy * i);
        }
    }
};

int TileMap::script_castShadows(lua_State* L)
{
    TileMap* const tileMap = TileMap::checkUserdata(L, 1);
    TileMask* const tileMask = TileMask::checkUserdata(L, 2);
    const int x = luaL_checkinteger(L, 3);
    const int y = luaL_checkinteger(L, 4);
    const int r = luaL_checkinteger(L, 5);
    const int mode = luaL_optinteger(L, 6, 1);

    const int cols = tileMap->m_cols;
    const int rows = tileMap->m_rows;

    luaL_argcheck(L, tileMap->m_index != nullptr, 1, "must have TileIndex");
    luaL_argcheck(L, cols == tileMask->getCols(), 2, "width doesn't match");
    luaL_argcheck(L, rows == tileMask->getRows(), 2, "height doesn't match");
    luaL_argcheck(L, (x >= 0 && x < cols), 3, "x is out of bounds");
    luaL_argcheck(L, (y >= 0 && y < rows), 4, "y is out of bounds");

    tileMask->fillMask(0);
    tileMask->setMask(x, y, 255);
    tileMap->m_debug.clear(); // HACK remove

    ShadowVector rightShadows(mode), bottomShadows(mode), leftShadows(mode), topShadows(mode);
    std::vector<int> visible(r);

#define SHADOW_TIMING
#ifdef SHADOW_TIMING
    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
#endif

    for (int i = 1; i <= r; ++i)
    {
        const int yl = y - i;
        const int yh = y + i;
        const int xl = x - i;
        const int xh = x + i;
        const int ylb = std::max(yl, 0);
        const int yhb = std::min(yh, rows - 1);
        const int xlb = std::max(xl, 0);
        const int xhb = std::min(xh, cols - 1);

        const int xlbi = std::max(-i, -x);
        const int xhbi = std::min(i, cols - 1 - x);
        const int ylbi = std::max(-i, -y);
        const int yhbi = std::min(i, rows - 1 - y);

        // TODO clamp to dx*dx + dy*dy <= r*r as well?

        if (xh == xhb)
        {
            //rightShadows.getVisible(i, ylb - y, yhb - y, visible);
            rightShadows.getVisible(i, ylbi, yhbi, visible);

            for (auto& yi : visible)
            {
                tileMask->setMask(xh, y + yi, 255);

                if (!tileMap->isFlagSet(xh, y + yi, TileIndex::VisionBlocking))
                    continue;

                rightShadows.castShadow(i, yi);
            }

            rightShadows.debug(x, y, 1, 0, i, tileMap->m_debug); // HACK remove
        }

        // TODO refactor with above
        if (yh == yhb)
        {
            bottomShadows.getVisible(i, xlbi, xhbi, visible);

            for (auto& xi : visible)
            {
                tileMask->setMask(x + xi, yh, 255);

                if (!tileMap->isFlagSet(x + xi, yh, TileIndex::VisionBlocking))
                    continue;

                bottomShadows.castShadow(i, xi);
            }

            bottomShadows.debug(x, y, 0, 1, i, tileMap->m_debug); // HACK remove
        }

        if (xl == xlb)
        {
            leftShadows.getVisible(i, ylbi, yhbi, visible);

            for (auto& yi : visible)
            {
                tileMask->setMask(xl, y + yi, 255);

                if (!tileMap->isFlagSet(xl, y + yi, TileIndex::VisionBlocking))
                    continue;

                leftShadows.castShadow(i, yi);
            }

            leftShadows.debug(x, y, -1, 0, i, tileMap->m_debug); // HACK remove
        }

        if (yl == ylb)
        {
            topShadows.getVisible(i, xlbi, xhbi, visible);

            for (auto& xi : visible)
            {
                tileMask->setMask(x + xi, yl, 255);

                if (!tileMap->isFlagSet(x + xi, yl, TileIndex::VisionBlocking))
                    continue;

                topShadows.castShadow(i, xi);
            }

            topShadows.debug(x, y, 0, -1, i, tileMap->m_debug); // HACK remove
        }
    }

#ifdef SHADOW_TIMING
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2 - t1);
    fprintf(stderr, "shadow casting took %lld usec\n", int64_t(time_span.count()));
#endif

    return 0;
}
