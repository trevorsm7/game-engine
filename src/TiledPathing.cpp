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

bool TiledPathing::visitNode(int node, int next, int end, std::vector<Node>& graph, std::queue<int>& toVisit)
{
    if (graph[next].valid)
    {
        graph[next].from = node;
        graph[next].valid = false;
        graph[next].weight = graph[node].weight + 1;

#if 0
        // HACK debug visualization
        m_points.reserve(m_points.size() + 5);
        m_points.push_back(2);
        int width = m_tilemap->getCols();
        m_points.push_back(node % width + 0.4f);
        m_points.push_back(node / width + 0.4f);
        m_points.push_back(next % width + 0.4f);
        m_points.push_back(next / width + 0.4f);
#endif

        if (next == end)
            return true;

        toVisit.push(next);
    }
    return false;
}

bool TiledPathing::findPath(int x1, int y1, int x2, int y2, int& xOut, int& yOut)
{
    if (!m_tilemap)
        return false;

    if (!m_tilemap->getTileSet())
        return false;

    const int width = m_tilemap->getCols();
    const int height = m_tilemap->getRows();
    const int size = width * height;

    if (x1 < 0 || x1 >= width || y1 < 0 || y1 >= height ||
        x2 < 0 || x2 >= width || y2 < 0 || y2 >= height)
        return false;

    const int src = x1 + y1 * width;
    const int dst = x2 + y2 * width;
    if (m_tilemap->isFlagSet(src, TileSet::MoveBlocking) ||
        m_tilemap->isFlagSet(dst, TileSet::MoveBlocking))
        return false;

    if (src == dst)
    {
        xOut = x1;
        yOut = y1;
        return true;
    }

//#define PATH_TIMING
#ifdef PATH_TIMING
    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
#endif

    std::vector<Node> graph(size);
    for (int i = 0; i < size; ++i)
    {
        //graph[i].weight = std::numeric_limits<int>::max();
        graph[i].valid = !m_tilemap->isFlagSet(i, TileSet::MoveBlocking);
    }

    // Start at destination
    std::queue<int> toVisit;
    toVisit.push(dst);
    graph[dst].weight = 1;

    while (!toVisit.empty())
    {
        int node = toVisit.front();
        assert(node != src);
        toVisit.pop();

        int y = node / width;
        int x = node % width;

        // At each level, alternate between branching priority
        if (graph[node].weight & 1)
        {
            if (x+1 < width && visitNode(node, node+1, src, graph, toVisit)) break;
            if (x-1 >= 0 && visitNode(node, node-1, src, graph, toVisit)) break;
            if (y+1 < height && visitNode(node, node+width, src, graph, toVisit)) break;
            if (y-1 >= 0 && visitNode(node, node-width, src, graph, toVisit)) break;
        }
        else
        {
            if (y-1 >= 0 && visitNode(node, node-width, src, graph, toVisit)) break;
            if (y+1 < height && visitNode(node, node+width, src, graph, toVisit)) break;
            if (x-1 >= 0 && visitNode(node, node-1, src, graph, toVisit)) break;
            if (x+1 < width && visitNode(node, node+1, src, graph, toVisit)) break;
        }
    }

#ifdef PATH_TIMING
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2 - t1);
    fprintf(stderr, "pathfinding took %lld usec\n", int64_t(time_span.count()));
#endif

    // If node still valid, it wasn't visited
    if (graph[src].valid != false)
        return false;

#if 0
    // HACK debug visualization
    const int pathLength = graph[src].weight;
    m_points.reserve(1 + pathLength * 2);
    m_points.push_back(pathLength);

    int node = src;
    while (true)
    {
        m_points.push_back(node % width + 0.6f);
        m_points.push_back(node / width + 0.6f);
        if (node == dst)
            break;
        node = graph[node].from;
    }
#endif

    int next = graph[src].from;
    xOut = next % width;
    yOut = next / width;
    return true;
}

void TiledPathing::construct(lua_State* L)
{
    getChildOpt(L, 2, "tilemap", m_tilemap);
}

void TiledPathing::clone(lua_State* L, TiledPathing* source)
{
    copyChild(L, m_tilemap, source->m_tilemap);
}

void TiledPathing::serialize(lua_State* L, Serializer* serializer, ObjectRef* ref)
{
    serializer->serializeMember(ref, "", "tilemap", "setTilemap", L, m_tilemap);
}

int TiledPathing::script_getTileMap(lua_State* L)
{
    TiledPathing* graphics = TiledPathing::checkUserdata(L, 1);
    return pushMember(L, graphics->m_tilemap);
}

int TiledPathing::script_setTileMap(lua_State* L)
{
    TiledPathing* graphics = TiledPathing::checkUserdata(L, 1);
    graphics->setChild(L, 2, graphics->m_tilemap);
    return 0;
}
