#pragma once

#include "IPathing.hpp"

#include <vector>

class TileMap;

class TiledPathing : public TUserdata<TiledPathing, IPathing>
{
    struct Node
    {
        int weight;
        bool valid;
    };

    void pushNode(int node, int depth, std::vector<int>& visit, bool& foundCheaper)
    {
        if (m_graph[node].valid)
        {
            if (m_graph[node].weight > depth+1)
            {
                m_graph[node].weight = depth+1;
                visit.push_back(node);
            }
            else if (m_graph[node].weight < depth-1)
                foundCheaper = true;
        }
    }

private:
    TileMap* m_tilemap;
    int m_width, m_height;
    std::vector<Node> m_graph;
    std::vector<float> m_points;

    TiledPathing(): m_tilemap(nullptr) {}

public:
    ~TiledPathing() override {}

    void update(float delta) override;
    void render(IRenderer* renderer) override;

    bool findPath(int x1, int y1, int x2, int y2, int& xOut, int& yOut) override;

private:
    void rebuildGraph();
    void setTileMap(lua_State* L, int index);

private:
    friend class TUserdata<TiledPathing, IPathing>;
    void construct(lua_State* L);
    void clone(lua_State* L, TiledPathing* source);
    //void destroy(lua_State* L) {}
    void serialize(lua_State* L, Serializer* serializer, ObjectRef* ref);

    static int script_getTileMap(lua_State* L);
    static int script_setTileMap(lua_State* L);

    static constexpr const char* const CLASS_NAME = "TiledPathing";
    static constexpr const luaL_Reg METHODS[] =
    {
        {"getTileMap", script_getTileMap},
        {"setTileMap", script_setTileMap},
        {nullptr, nullptr}
    };
};
