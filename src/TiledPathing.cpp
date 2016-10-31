#include "TiledPathing.hpp"
#include "TileMap.hpp"
#include "Serializer.hpp"
#include "IRenderer.hpp"

#include <limits>
#include <chrono>

const luaL_Reg TiledPathing::METHODS[];

void TiledPathing::update(float delta)
{
    m_points.clear();
}

void TiledPathing::render(IRenderer* renderer)
{
    if (!m_points.empty())
        renderer->drawLines(m_points);
}

// TODO remove this or make it automatic
void TiledPathing::rebuildGraph()
{
    if (!m_tilemap) return;
    TileIndex* index = m_tilemap->getTileIndex();
    if (!index) return;

    m_width = m_tilemap->getCols();
    m_height = m_tilemap->getRows();
    int size = m_width * m_height;
    m_graph.resize(size);

    for (int i = 0; i < size; ++i)
        m_graph[i].valid = !index->isCollidable(m_tilemap->getIndex(i));
}

bool TiledPathing::findPath(int x1, int y1, int x2, int y2, int& xOut, int& yOut)
{
    if (x1 < 0 || x1 >= m_width || y1 < 0 || y1 >= m_height ||
        x2 < 0 || x2 >= m_width || y2 < 0 || y2 >= m_height)
        return false;

    int src = x1 + y1 * m_width;
    int dst = x2 + y2 * m_width;
    if (!m_graph[src].valid || !m_graph[dst].valid)
        return false;

    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (int i = 0; i < m_width * m_height; ++i)
        m_graph[i].weight = std::numeric_limits<int>::max();

    std::vector<int> allNodes;

    int node = src;
    int depth = 0;
    std::vector<int> stack;
    std::vector<int> counts;
    std::vector<int> visit;
    while (node != dst)
    {
        int y = node / m_width;
        int x = node % m_width;

        if (depth <= m_graph[node].weight)
        {
            m_graph[node].weight = depth;
            bool foundCheaper = false;
            int dx = x2 - x;
            int dy = y2 - y;

            allNodes.push_back(node); // HACK debug

            int before = visit.size();
            if (abs(dx) >= abs(dy))
            {
                dx = (dx >= 0) ? 1 : -1;
                dy = (dy >= 0) ? 1 : -1;
                if (x-dx >= 0 && x-dx < m_width) pushNode(node-dx, depth, visit, foundCheaper);
                if (y-dy >= 0 && y-dy < m_height) pushNode(node-dy*m_width, depth, visit, foundCheaper);
                if (y+dy >= 0 && y+dy < m_height) pushNode(node+dy*m_width, depth, visit, foundCheaper);
                if (x+dx >= 0 && x+dx < m_width) pushNode(node+dx, depth, visit, foundCheaper);
            }
            else
            {
                dx = (dx >= 0) ? 1 : -1;
                dy = (dy >= 0) ? 1 : -1;
                if (y-dy >= 0 && y-dy < m_height) pushNode(node-dy*m_width, depth, visit, foundCheaper);
                if (x-dx >= 0 && x-dx < m_width) pushNode(node-dx, depth, visit, foundCheaper);
                if (x+dx >= 0 && x+dx < m_width) pushNode(node+dx, depth, visit, foundCheaper);
                if (y+dy >= 0 && y+dy < m_height) pushNode(node+dy*m_width, depth, visit, foundCheaper);
            }
            int pushed = visit.size() - before;

            if (pushed > 0)
            {
                if (!foundCheaper)
                {
                    stack.push_back(node);
                    counts.push_back(pushed-1);
                    node = visit.back();
                    visit.pop_back();
                    ++depth;
                    continue;
                }
                else
                    visit.resize(before);
            }
        }

        while (!counts.empty())
        {
            allNodes.push_back(stack.back()); // HACK debug

            if (counts.back() > 0)
            {
                assert(!visit.empty());
                node = visit.back();
                visit.pop_back();
                --(counts.back());
                break;
            }

            stack.pop_back();
            counts.pop_back();
            --depth;
        }

        if (!counts.empty())
            continue;

        fprintf(stderr, "ran out of nodes\n");
        return false;
    }
    stack.push_back(dst);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2 - t1);
    //fprintf(stderr, "pathfinding took %lld usec\n", int64_t(time_span.count()));

    if (allNodes.size() >= 2)
    {
        float step = 0.2f / allNodes.size();
        float offset = 0.5f;

        m_points.reserve(m_points.size() + allNodes.size() * 2 + 1);
        m_points.push_back(allNodes.size());
        for (auto& i : allNodes)
        {
            m_points.push_back(i % m_width + offset);
            m_points.push_back(i / m_width + offset);
            offset += step;
        }
    }

    //lua_createtable(L, stack.size(), 0);
    if (stack.size() >= 2)
    {
        m_points.reserve(m_points.size() + stack.size() * 2 + 1);
        m_points.push_back(stack.size());
        //for (auto& i : stack)
        for (auto it = stack.rbegin(); it != stack.rend(); ++it)
        {
            m_points.push_back(*it % m_width + 0.4f);
            m_points.push_back(*it / m_width + 0.4f);
        }

        node = stack[1];
        xOut = node % m_width;
        yOut = node / m_width;
        return true;
    }

    xOut = x2;
    yOut = y2;
    return true;
}

void TiledPathing::setTileMap(lua_State* L, int index)
{
    TileMap* tilemap = TileMap::checkUserdata(L, index);

    // Do nothing if we already own the component
    if (m_tilemap == tilemap)
        return;

    // Clear old component first
    if (m_tilemap != nullptr)
        releaseChild(L, m_tilemap);

    // Add component to new actor
    acquireChild(L, tilemap, index);
    m_tilemap = tilemap;

    // TODO rebuild pathing graph
    rebuildGraph();
}

void TiledPathing::construct(lua_State* L)
{
    lua_pushliteral(L, "tilemap");
    if (lua_rawget(L, 2) != LUA_TNIL)
    {
        setChild(L, m_tilemap, -1);
        rebuildGraph();
    }
    lua_pop(L, 1);
}

void TiledPathing::clone(lua_State* L, TiledPathing* source)
{
    if (source->m_tilemap)
    {
        // Don't need to clone TileMap; just copy
        //source->m_tilemap->pushClone(L);
        source->m_tilemap->pushUserdata(L);
        setChild(L, m_tilemap, -1);
        lua_pop(L, 1);
        rebuildGraph();
    }
}

void TiledPathing::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    if (m_tilemap)
    {
        m_tilemap->pushUserdata(L);
        serializer->serializeMember(ref, "", "tilemap", "setTilemap", L, -1);
        lua_pop(L, 1);
    }
}

int TiledPathing::script_getTileMap(lua_State* L)
{
    TiledPathing* graphics = TiledPathing::checkUserdata(L, 1);
    return pushMember(L, graphics->m_tilemap);
}

int TiledPathing::script_setTileMap(lua_State* L)
{
    TiledPathing* graphics = TiledPathing::checkUserdata(L, 1);
    graphics->setChild(L, graphics->m_tilemap, 2);
    graphics->rebuildGraph();
    return 0;
}
