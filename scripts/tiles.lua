game = Canvas({20, 20}, false)

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
    position = {-1, -1},
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
    data =
    {
        0, 1, 0,
        1, 1, 1,
        0, 1, 0
    }
}

--player = Actor{sprite="hero.tga", collider=true}
player = Actor
{
    graphics = TiledGraphics{tilemap=heropad},
    collider = TiledCollider{tilemap=heropad},
    position = {-2, -2},
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
registerControl("down", player:keyDown(player.move, {0, -1}))
registerControl("up", player:keyDown(player.move, {0, 1}))

nerd = Actor
{
    graphics = SpriteGraphics{sprite="nerd.tga"},
    collider = AabbCollider{},
    position = {4, 4}
}
game:addActor(nerd)
