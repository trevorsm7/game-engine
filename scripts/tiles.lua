game = Canvas
{
    size = {20, 20},
    fixed = false
}

local map = TileMap
{
    index = TileIndex
    {
        sprite = "tiles.tga",
        size = {2, 2},
        data =
        {
            0, 0,
            1, 1
        }
    },
    size = {7, 7},
    data =
    {
        3, 3, 4, 3, 3, 3, 3,
        3, 1, 1, 1, 1, 1, 3,
        4, 1, 2, 1, 1, 1, 3,
        3, 1, 1, 1, 1, 3, 0,
        3, 3, 1, 3, 3, 0, 0,
        0, 4, 1, 4, 0, 0, 0,
        0, 3, 1, 3, 0, 0, 0
    }
}

tiles = Actor
{
    graphics = TiledGraphics{tilemap=map},
    collider = TiledCollider{tilemap=map},
    position = {0, 0},
    scale = {7, 7} -- map:getSize()
}
game:addActor(tiles)
game:setCenter(2, 2)

local heropad = TileMap
{
    index = TileIndex
    {
        sprite = "hero.tga",
        size = {1, 1},
        data = {1}
    },
    size = {3, 3},
    --[[data =
    {
        0, 1, 0,
        1, 1, 1,
        0, 1, 0
    }--]]
}
heropad:setTiles(0, 0, 3, 3, 0)
heropad:setTiles(0, 1, 3, 1, 1)
heropad:setTiles(1, 0, 1, 3, 1)

--player = Actor{sprite="hero.tga", collider=true}
player = Actor
{
    graphics = TiledGraphics{tilemap=heropad},
    collider = TiledCollider{tilemap=heropad},
    position = {1, 1},
    scale = {3, 3} -- heropad:getSize()
}
game:addActor(player)

-- generate a callback function to move the player in a direction
function player:keyDown(method, arg)
    return function(down)
        if down then method(self, arg) end
    end
end

function player:move(dir)
    local canvas = self:getCanvas()
    if not canvas then return end

    local x, y = self:getPosition()
    x = x + dir[1] * .5; y = y + dir[2] * .5
    --local hit = canvas:getCollision(x + 0.5, y + 0.5)
    --local hit = canvas:getCollision(x, y)

    if not hit then
        self:setPosition(x, y)
    end
end

function player:update(delta)
    if self:testCollision(0, 0) then
        self:getGraphics():setColor{1, .5, .5}
    else
        self:getGraphics():setColor{1, 1, 1}
    end
end

registerControl("left", player:keyDown(player.move, {-1, 0}))
registerControl("right", player:keyDown(player.move, {1, 0}))
registerControl("down", player:keyDown(player.move, {0, 1}))
registerControl("up", player:keyDown(player.move, {0, -1}))

nerd = Actor
{
    graphics = SpriteGraphics{sprite="nerd.tga"},
    collider = AabbCollider{},
    position = {5, 5},
    members = {test = "hello", val = 5}
}
game:addActor(nerd)

---[[
tiles:getGraphics().herp = 'derp'
tiles.str = "hello"
tiles.num = 5
tiles[3] = "num key"
tiles.bool = true
tiles[false] = "bool key"
tiles.asdf = nil -- nop!
tiles.table = {1, 2, 3}
tiles.player = player
tiles.nerd = nerd
--]]

--tiles.cycle = tiles
tiles.cycle = {cycle = tiles}

--[[
e = {}
d = {e=e}
c = {d=d}
b = {e=e}
a = {b=b, c=c}
e.a = a
tiles.a = a
--]]

--[[
tiles[1] = nerd:getGraphics()
tiles[1].cycle = nerd
tiles[2] = nerd
--]]

--tiles:serialize()
serialize()
