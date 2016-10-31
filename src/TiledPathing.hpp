#pragma once

#include "IPathing.hpp"

#include <vector>
#include <queue>

class TileMap;

class TiledPathing : public TUserdata<TiledPathing, IPathing>
{
    struct Node
    {
        int weight;
        int from;
        bool valid;
    };

    inline bool visitNode(int node, int next, int end, std::vector<Node>& graph, std::queue<int>& toVisit);

private:
    TileMap* m_tilemap;
    std::vector<float> m_points;

    TiledPathing(): m_tilemap(nullptr) {}

public:
    ~TiledPathing() override {}

    void update(float delta) override;
    void render(IRenderer* renderer) override;

    bool findPath(int x1, int y1, int x2, int y2, int& xOut, int& yOut) override;

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
