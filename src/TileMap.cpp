#include "TileMap.h"
#include "ResourceManager.h"

#include <sstream>
#include <iterator>
#include <algorithm>

TileMapPtr TileMap::loadTileMap(ResourceManager& manager, const std::string& filename)
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
}

TileIndexPtr TileIndex::loadTileIndex(ResourceManager& manager, const std::string& filename)
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
}
