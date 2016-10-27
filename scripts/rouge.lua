-- use a global timer for managing turns
local gameTime = 0

local function newPlayer(canvas, x, y)
    local player = Actor
    {
        graphics = SpriteGraphics{sprite="hero.tga"},
        collider = AabbCollider{},
        transform = {position = {x, y}},
        members =
        {
            player = true,
            stepTime = 1,
            hp = 3
        }
    }
    canvas:addActor(player)
    canvas:setCenter(player)

    function player:update(delta)
        local canvas = self:getCanvas()
        if canvas then canvas:setCenter(self) end
    end

    function player:idle()
        local canvas = self:getCanvas()
        if not canvas then return end

        gameTime = gameTime + self.stepTime
        pf:clearPath()
    end

    function player:attack(target)
        print("Take that, nerd!")
        target:getCanvas():removeActor(target)
    end

    function player:move(dir)
        local canvas = self:getCanvas()
        if not canvas then return end

        local x, y = self:getPosition()
        x = x + dir[1]; y = y + dir[2]
        --local hit = canvas:getCollision(x + 0.5, y + 0.5)
        local hit = canvas:getCollision(x, y)

        if not hit then
            self:setPosition(x, y)
            gameTime = gameTime + self.stepTime
        elseif hit.enemy then
            self:attack(hit)
            gameTime = gameTime + self.stepTime
        elseif hit.interact then
            gameTime = gameTime + hit:interact(self)
        end
        pf:clearPath()
    end

    return player
end

local function newNerd(canvas, x, y)
    local nerd = Actor
    {
        graphics = SpriteGraphics{sprite="nerd.tga"},
        collider = AabbCollider{},
        transform = {position = {x, y}},
        members =
        {
            enemy = true,
            stepTime = 1,
            time = gameTime
        }
    }
    canvas:addActor(nerd)

    function nerd:attack(target)
        local hp = target.hp
        hp = hp - 1
        target.hp = hp
        print("Ouch! HP left: "..hp)
        if hp <= 0 then
            target:getCanvas():removeActor(target)
        end
    end

    function nerd:update(delta)
        local canvas = self:getCanvas()
        if not canvas or not player or player:getCanvas() ~= canvas then return end

        -- TODO handle actions that can have different step times
        while self.stepTime <= gameTime - self.time do
            local x, y = self:getPosition()
            local px, py = player:getPosition()
            local nx, ny = pf:findPath(x, y, px, py) -- TODO cache multiple path steps so we don't need to call repeatedly
            -- TODO need pathfinding to take Actors into account so we path around them

            local hit = canvas:getCollision(nx, ny)
            if not hit then
                self:setPosition(nx, ny)
                self.time = self.time + self.stepTime
            elseif hit == player then --(nx == px and ny == py)
                self:attack(player)
                self.time = self.time + self.stepTime
            elseif hit.interact then
                self.time = self.time + hit:interact(self)
            else
                -- no moves available; perform a wait to drain time
                --self.time = self.time + self.stepTime
                self.time = gameTime - (gameTime - self.time) % self.stepTime
                break
            end
        end
    end

    return nerd
end

local function newDoor(canvas, map, x, y)
    map:setTiles(x, y, 1, 1, 1)

    local door = Actor
    {
        graphics = SpriteGraphics{sprite="door.tga"},
        collider = AabbCollider{},
        transform = {position = {x, y}},
        members =
        {
            open = false,
            interact = function(self, actor)
                local canvas = self:getCanvas()
                if canvas then
                    if self.open then
                        self:setScale(1, 1)
                        self:getCollider():setCollidable(true)
                        self.open = false
                        return 1 -- time to close door
                    else
                        self:setScale(0.1, 1)
                        self:getCollider():setCollidable(false)
                        self.open = true
                        return 1 -- time to open door
                    end
                end
                return 0
            end
        }
    }
    canvas:addActor(door)

    return door
end

local function newRoom(map, x, y, w, h)
    -- Fill walls
    map:setTiles(x, y, w, 1, 3)
    map:setTiles(x, y+h-1, w, 1, 3)
    map:setTiles(x, y+1, 1, h-2, 3)
    map:setTiles(x+w-1, y+1, 1, h-2, 3)
    local nDoodads = math.random((w+h-4)//4, (w+h-4)//2)
    while nDoodads > 0 do
        local side = math.random(0, 3)
        if side == 0 then
            map:setTiles(x, math.random(y+1, y+h-2), 1, 1, 4)
        elseif side == 1 then
            map:setTiles(x+w-1, math.random(y+1, y+h-2), 1, 1, 4)
        elseif side == 2 then
            map:setTiles(math.random(x+1, x+w-2), y, 1, 1, 4)
        else
            map:setTiles(math.random(x+1, x+w-2), y+h-1, 1, 1, 4)
        end
        nDoodads = nDoodads - 1
    end

    -- Fill floor
    map:setTiles(x+1, y+1, w-2, h-2, 1)
    local nDoodads = math.random(0, (w-2)*(h-2)//9)
    while nDoodads > 0 do
        map:setTiles(math.random(x+1, x+w-2), math.random(y+1, y+h-2), 1, 1, 2)
        nDoodads = nDoodads - 1
    end
end

spawnPoints = {{3,2}, {7, 3}, {13, 2}, {2,3}, {8, 2}, {12, 3}}

local game = Canvas
{
    size = {20, 20},
    fixed = false,
    members =
    {
        time = gameTime,
        spawnTime = 10,
        update = function(self)
            if self.spawnTime <= gameTime - self.time then
                self.time = self.time + self.spawnTime
                local idx = math.random(1, #spawnPoints)
                newNerd(self, table.unpack(spawnPoints[idx]))
                if self.spawnTime > 2 then
                    self.spawnTime = self.spawnTime - 1
                end
            end
        end
    }
}
addCanvas(game)

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
    size = {16, 11}
}

pf = TiledPathing{};

tiles = Actor
{
    graphics = TiledGraphics{tilemap=map},
    collider = TiledCollider{tilemap=map},
    pathing = pf,
    transform = {position = {0, 0}, scale = {map:getSize()}},
    layer = -1
}
game:addActor(tiles)

newRoom(map, 0, 0, 6, 6)
newRoom(map, 5, 0, 6, 6)
newRoom(map, 10, 0, 6, 6)
newRoom(map, 0, 5, 6, 6)
newRoom(map, 5, 5, 6, 6)
newRoom(map, 10, 5, 6, 6)

newDoor(game, map, 5, 2)
newDoor(game, map, 7, 5)
newDoor(game, map, 5, 8)
newDoor(game, map, 10, 7)
newDoor(game, map, 12, 5)

pf:setTileMap(map)

newNerd(game, 13, 3)
newNerd(game, 3, 8)

player = newPlayer(game, 1, 1)

function keyDown(actor, method, arg)
    return function(down)
        if down and actor then actor[method](actor, arg) end
    end
end

registerControl("left", keyDown(player, "move", {-1, 0}))
registerControl("right", keyDown(player, "move", {1, 0}))
registerControl("down", keyDown(player, "move", {0, 1}))
registerControl("up", keyDown(player, "move", {0, -1}))
registerControl("action", keyDown(player, "idle"))
