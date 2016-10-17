#include "Pathfinding.hpp"
#include "IRenderer.hpp"
#include "TileMap.hpp"

#include <limits>
#include <chrono>

const luaL_Reg Pathfinding::METHODS[];

void Pathfinding::render(IRenderer* renderer)
{
    if (!m_points.empty())
        renderer->drawLines(m_points);
}

int Pathfinding::script_addTiles(lua_State* L)
{
    // Validate function arguments
    Pathfinding* pathfinding = Pathfinding::checkUserdata(L, 1);
    TileMap* tiles = TileMap::checkUserdata(L, 2);
    TileIndex* index = tiles->getTileIndex();
    luaL_argcheck(L, (index != nullptr), 2, "TileMap must have valid TileIndex\n");

    std::vector<Node>& graph = pathfinding->m_graph;

    const int cols = tiles->getCols();
    const int rows = tiles->getRows();

    pathfinding->m_width = cols;
    pathfinding->m_height = rows;
    int size = cols * rows;
    graph.resize(size);

    for (int i = 0; i < size; ++i)
        graph[i].valid = !index->isCollidable(tiles->getIndex(i));

    return 0;
}

int Pathfinding::script_findPath(lua_State* L)
{
    // Validate function arguments
    Pathfinding* pathfinding = Pathfinding::checkUserdata(L, 1);
    /*float x1 = static_cast<float>(luaL_checknumber(L, 2));
    float y1 = static_cast<float>(luaL_checknumber(L, 3));
    float x2 = static_cast<float>(luaL_checknumber(L, 4));
    float y2 = static_cast<float>(luaL_checknumber(L, 5));*/
    const int x1 = luaL_checkinteger(L, 2);
    const int y1 = luaL_checkinteger(L, 3);
    const int x2 = luaL_checkinteger(L, 4);
    const int y2 = luaL_checkinteger(L, 5);

    std::vector<Node>& graph = pathfinding->m_graph;
    std::vector<float>& points = pathfinding->m_points;
    const int width = pathfinding->m_width;
    const int height = pathfinding->m_height;

    points.clear();

    if (x1 < 0 || x1 >= width || y1 < 0 || y1 >= height ||
        x2 < 0 || x2 >= width || y2 < 0 || y2 >= height)
        return 0;

    int src = x1 + y1 * width;
    int dst = x2 + y2 * width;
    if (!graph[src].valid || !graph[dst].valid)
        return 0;

    using namespace std::chrono;
    high_resolution_clock::time_point t1 = high_resolution_clock::now();

    for (int i = 0; i < width * height; ++i)
        graph[i].weight = std::numeric_limits<int>::max();

    std::vector<int> allNodes;

    int node = src;
    int depth = 0;
    std::vector<int> stack;
    std::vector<int> counts;
    std::vector<int> visit;
    while (node != dst)
    {
        int y = node / width;
        int x = node % width;

        if (depth <= graph[node].weight)
        {
            graph[node].weight = depth;
            bool foundCheaper = false;
            int dx = x2 - x;
            int dy = y2 - y;

            allNodes.push_back(node); // HACK debug

            int before = visit.size();
            if (abs(dx) >= abs(dy))
            {
                dx = (dx >= 0) ? 1 : -1;
                dy = (dy >= 0) ? 1 : -1;
                if (x-dx >= 0 && x-dx < width) pushNode(graph, node-dx, depth, visit, foundCheaper);
                if (y-dy >= 0 && y-dy < height) pushNode(graph, node-dy*width, depth, visit, foundCheaper);
                if (y+dy >= 0 && y+dy < height) pushNode(graph, node+dy*width, depth, visit, foundCheaper);
                if (x+dx >= 0 && x+dx < width) pushNode(graph, node+dx, depth, visit, foundCheaper);
            }
            else
            {
                dx = (dx >= 0) ? 1 : -1;
                dy = (dy >= 0) ? 1 : -1;
                if (y-dy >= 0 && y-dy < height) pushNode(graph, node-dy*width, depth, visit, foundCheaper);
                if (x-dx >= 0 && x-dx < width) pushNode(graph, node-dx, depth, visit, foundCheaper);
                if (x+dx >= 0 && x+dx < width) pushNode(graph, node+dx, depth, visit, foundCheaper);
                if (y+dy >= 0 && y+dy < height) pushNode(graph, node+dy*width, depth, visit, foundCheaper);
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
        return 0;
    }
    stack.push_back(dst);

    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    microseconds time_span = duration_cast<microseconds>(t2 - t1);
    //fprintf(stderr, "pathfinding took %lld usec\n", int64_t(time_span.count()));

    if (allNodes.size() >= 2)
    {
        float step = 0.2f / allNodes.size();
        float offset = 0.5f;

        points.reserve(points.size() + allNodes.size() * 2 + 1);
        points.push_back(allNodes.size());
        for (auto& i : allNodes)
        {
            points.push_back(i % width + offset);
            points.push_back(i / width + offset);
            offset += step;
        }
    }

    //lua_createtable(L, stack.size(), 0);
    if (stack.size() >= 2)
    {
        points.reserve(points.size() + stack.size() * 2 + 1);
        points.push_back(stack.size());
        //for (auto& i : stack)
        for (auto it = stack.rbegin(); it != stack.rend(); ++it)
        {
            points.push_back(*it % width + 0.4f);
            points.push_back(*it / width + 0.4f);
        }

        node = stack[1];
        lua_pushinteger(L, node % width);
        lua_pushinteger(L, node / width);
        return 2;
    }

    fprintf(stderr, "stack too small?\n");

    return 0;
}
