map = TileMap
{
    tileset = TileSet
    {
        filename = "tiles.tga",
        size = {2, 3},
        data =
        {
            0, 0,
            3, 3,
            2, 0
        }
    },
    size = {20, 15}
}

map:setTiles(0, 0, 20, 1, 3)
map:setTiles(0, 14, 20, 1, 3)
map:setTiles(0, 1, 1, 13, 3)
map:setTiles(19, 1, 1, 13, 3)
map:setTiles(1, 1, 18, 13, 1)

map:setTiles(16, 1, 1, 3, 3)
map:setTiles(17, 3, 2, 1, 3)

map:setTiles(5, 5, 10, 1, 3)
map:setTiles(5, 10, 10, 1, 3)

local game = Canvas
{
    camera = Camera2D
    {
        size = {20, 15},
        fixed = true
    }
}
addCanvas(game)

pawnA = Actor
{
    graphics = SpriteGraphics{sprite = "hero.tga"},
    transform = {position = {1, 1}}
}
game:addActor(pawnA)

pawnB = Actor
{
    graphics = SpriteGraphics{sprite = "nerd.tga"},
    transform = {position = {18, 13}}
}
game:addActor(pawnB)

pf = TiledPathing{tilemap=map};

local mapActor = Actor
{
    graphics = TiledGraphics{tilemap=map},
    collider = TiledCollider{tilemap=map},
    pathing = pf,
    transform = {position = {0, 0}},
    layer = -1
}
game:addActor(mapActor)

function moveFunc(actor, dx, dy)
    return function(down)
        if down then
            local x, y = actor:getPosition()
            actor:setPosition(x+dx, y+dy)
            local x1, y1 = pawnB:getPosition()
            local x2, y2 = pawnA:getPosition()
            pf:clearPath()
            pf:findPath(x1, y1, x2, y2)
        end
    end
end

registerControl("left", moveFunc(pawnA, -1, 0))
registerControl("right", moveFunc(pawnA, 1, 0))
registerControl("down", moveFunc(pawnA, 0, 1))
registerControl("up", moveFunc(pawnA, 0, -1))
registerControl("a", moveFunc(pawnB, -1, 0))
registerControl("d", moveFunc(pawnB, 1, 0))
registerControl("s", moveFunc(pawnB, 0, 1))
registerControl("w", moveFunc(pawnB, 0, -1))
