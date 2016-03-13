#ifndef __TILEMAP_H__
#define __TILEMAP_H__

#include "IResource.h"
#include "ResourceManager.h"

#include <memory>
#include <string>
#include <vector>

class TileMap;
typedef std::shared_ptr<TileMap> TileMapPtr;

class TileIndex;
typedef std::shared_ptr<TileIndex> TileIndexPtr;

class TileMap : public IResource
{
    std::string m_index;
    std::vector<int> m_map;
    int m_cols, m_rows;

protected:
    TileMap() {}

public:
    ~TileMap() override {}

    const std::string& getIndexFile() const {return m_index;}
    int getIndex(int i) const {if (inBounds(i)) return m_map[i]; return 0;}
    int getIndex(int x, int y) const {if (inBounds(x, y)) return m_map[y * m_cols + x]; return 0;}
    int getCols() const {return m_cols;}
    int getRows() const {return m_rows;}

    bool inBounds(int i) const {return i < m_cols * m_rows;}
    bool inBounds(int x, int y) const {return x < m_cols && y < m_rows;}

    static TileMapPtr loadTileMap(ResourceManager& manager, const std::string& filename);
};

class TileIndex : public IResource
{
    std::string m_image;
    std::vector<char> m_flags;
    int m_cols, m_rows;

protected:
    TileIndex() {}

public:
    ~TileIndex() override {}

    const std::string& getImageFile() const {return m_image;}
    bool getCollidable(int i) const {return inBounds(i) && m_flags[i] & 1;}
    int getCols() const {return m_cols;}
    int getRows() const {return m_rows;}

    bool inBounds(int i) const {return i < m_cols * m_rows;}

    static TileIndexPtr loadTileIndex(ResourceManager& manager, const std::string& filename);
};

#endif
