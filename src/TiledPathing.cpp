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

    TileIndex* tileIndex = m_tilemap->getTileIndex();
    if (!tileIndex)
        return false;

    const int width = m_tilemap->getCols();
    const int height = m_tilemap->getRows();
    const int size = width * height;

    if (x1 < 0 || x1 >= width || y1 < 0 || y1 >= height ||
        x2 < 0 || x2 >= width || y2 < 0 || y2 >= height)
        return false;

    const int src = x1 + y1 * width;
    const int dst = x2 + y2 * width;
    if (m_tilemap->isCollidable(src) || m_tilemap->isCollidable(dst))
        return false;

    if (src == dst)
    {
        xOut = x1;
        yOut = y1;
        return true;
    }

    //using namespace std::chrono;
    //high_resolution_clock::time_point t1 = high_resolution_clock::now();

    std::vector<Node> graph(size);
    for (int i = 0; i < size; ++i)
    {
        //graph[i].weight = std::numeric_limits<int>::max();
        graph[i].valid = !m_tilemap->isCollidable(i);
    }

    // Start at source
    std::queue<int> toVisit;
    toVisit.push(src);
    graph[src].weight = 1;

    while (!toVisit.empty())
    {
        int node = toVisit.front();
        assert(node != dst);
        toVisit.pop();

        int y = node / width;
        int x = node % width;
        int dx = x2 - x;
        int dy = y2 - y;

        // Prefer path facing destination
        if (abs(dx) >= abs(dy))
        {
            dx = (dx >= 0) ? 1 : -1;
            dy = (dy >= 0) ? width : -width;
            if (x+dx >= 0 && x+dx < width && visitNode(node, node+dx, dst, graph, toVisit)) break;
            if (node+dy >= 0 && node+dy < size && visitNode(node, node+dy, dst, graph, toVisit)) break;
            if (node-dy >= 0 && node-dy < size && visitNode(node, node-dy, dst, graph, toVisit)) break;
            if (x-dx >= 0 && x-dx < width && visitNode(node, node-dx, dst, graph, toVisit)) break;
        }
        else
        {
            dx = (dx >= 0) ? 1 : -1;
            dy = (dy >= 0) ? width : -width;
            if (node+dy >= 0 && node+dy < size && visitNode(node, node+dy, dst, graph, toVisit)) break;
            if (x+dx >= 0 && x+dx < width && visitNode(node, node+dx, dst, graph, toVisit)) break;
            if (x-dx >= 0 && x-dx < width && visitNode(node, node-dx, dst, graph, toVisit)) break;
            if (node-dy >= 0 && node-dy < size && visitNode(node, node-dy, dst, graph, toVisit)) break;
        }
    }

    // If node still valid, it wasn't visited
    if (graph[dst].valid != false)
        return false;

    //high_resolution_clock::time_point t2 = high_resolution_clock::now();
    //microseconds time_span = duration_cast<microseconds>(t2 - t1);
    //fprintf(stderr, "pathfinding took %lld usec\n", int64_t(time_span.count()));

    // TODO Walk forwards to build path
    const int pathLength = graph[dst].weight;
    m_points.reserve(1 + pathLength * 2);
    m_points.push_back(pathLength);
    m_points.push_back(dst % width + 0.6f);
    m_points.push_back(dst / width + 0.6f);

    // Walk backwards from destination to source
    int node = dst, prev = dst;
    for (int i = 1; i < pathLength; ++i)
    {
        prev = node;
        node = graph[node].from;
        m_points.push_back(node % width + 0.6f);
        m_points.push_back(node / width + 0.6f);
    }
    assert(graph[prev].from == src);
    assert(node == src);

    xOut = prev % width;
    yOut = prev / width;
    return true;
}

void TiledPathing::construct(lua_State* L)
{
    lua_pushliteral(L, "tilemap");
    if (lua_rawget(L, 2) != LUA_TNIL)
        setChild(L, m_tilemap, -1);
    lua_pop(L, 1);
}

void TiledPathing::clone(lua_State* L, TiledPathing* source)
{
    if (source->m_tilemap)
    {
        source->m_tilemap->pushUserdata(L); // NOTE don't need to clone
        setChild(L, m_tilemap, -1);
        lua_pop(L, 1);
    }
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
    graphics->setChild(L, graphics->m_tilemap, 2);
    return 0;
}
