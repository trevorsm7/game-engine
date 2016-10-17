#pragma once

#include "IUserdata.hpp"

#include <vector>

class IRenderer;

class Pathfinding : public TUserdata<Pathfinding>
{
    struct Node
    {
        int weight;
        bool valid;
    };

    static void pushNode(std::vector<Node>& graph, int node, int depth, std::vector<int>& visit, bool& foundCheaper)
    {
        if (graph[node].valid)
        {
            if (graph[node].weight > depth+1)
            {
                graph[node].weight = depth+1;
                visit.push_back(node);
            }
            else if (graph[node].weight < depth-1)
                foundCheaper = true;
        }
    }

private:
    std::vector<float> m_points;
    int m_width, m_height;
    std::vector<Node> m_graph;

    Pathfinding() {}

public:
    void render(IRenderer* renderer); // for debugging

private:
    friend class TUserdata<Pathfinding>;
    //void construct(lua_State* L);
    //void destroy(lua_State* L) {}
    //void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_addTiles(lua_State* L);
    static int script_findPath(lua_State* L);

    static constexpr const char* const CLASS_NAME = "Pathfinding";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"addTiles", script_addTiles},
        {"findPath", script_findPath},
        {nullptr, nullptr}
    };
};
