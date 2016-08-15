local singleTile = TileIndex
{
    sprite = "square.tga",
    size = {1, 1},
    data = {1}
}

-- TODO allow Actor/graphics/collider to rotate around given pivot point
tetrominoes =
{
    { -- I
        color = {1, 0, 0},
        TileMap
        {
            index = singleTile,
            size = {4, 4},
            data =
            {
                0, 1, 0, 0,
                0, 1, 0, 0,
                0, 1, 0, 0,
                0, 1, 0, 0
            }
        },
        TileMap
        {
            index = singleTile,
            size = {4, 4},
            data =
            {
                0, 0, 0, 0,
                1, 1, 1, 1,
                0, 0, 0, 0,
                0, 0, 0, 0
            }
        },
    },
    { -- J
        color = {1, 1, 0},
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 1, 0,
                0, 1, 0,
                1, 1, 0
            }
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 0, 0,
                1, 1, 1,
                0, 0, 1
            }
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 1, 1,
                0, 1, 0,
                0, 1, 0
            }
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                1, 0, 0,
                1, 1, 1,
                0, 0, 0
            }
        }
    },
    { -- L
        color = {1, 0, 1},
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 1, 0,
                0, 1, 0,
                0, 1, 1
            }
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 0, 1,
                1, 1, 1,
                0, 0, 0
            }
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                1, 1, 0,
                0, 1, 0,
                0, 1, 0
            }
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 0, 0,
                1, 1, 1,
                1, 0, 0
            }
        }
    },
    { -- O
        color = {0, 0, 1},
        TileMap
        {
            index = singleTile,
            size = {2, 2},
            data =
            {
                1, 1,
                1, 1
            }
        }
    },
    { -- S
        color = {0, 1, 1},
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 0, 0,
                0, 1, 1,
                1, 1, 0
            },
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                1, 0, 0,
                1, 1, 0,
                0, 1, 0
            }
        }
    },
    { -- T
        color = {0.5, 1, 0},
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 1, 0,
                1, 1, 1,
                0, 0, 0
            }
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 1, 0,
                1, 1, 0,
                0, 1, 0
            }
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 0, 0,
                1, 1, 1,
                0, 1, 0
            }
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 1, 0,
                0, 1, 1,
                0, 1, 0
            }
        }
    },
    { -- Z
        color = {1, 0.5, 0},
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 0, 0,
                1, 1, 0,
                0, 1, 1
            },
        },
        TileMap
        {
            index = singleTile,
            size = {3, 3},
            data =
            {
                0, 1, 0,
                1, 1, 0,
                1, 0, 0
            }
        }
    },
}

-- request a portait-oriented window
local screenSize = {10, 20}
setPortraitHint(true)

game = Canvas
{
    size = {screenSize[1] + 6, screenSize[2] + 1},
    fixed = true
}
game:setOrigin(-1, 0)

-- Create the TileMap that will represent all the fallen blocks
screenMap = TileMap
{
    index = singleTile,
    size = screenSize
}

-- We'll mainly be working with the TileMap, not the Actor
local screen = Actor
{
    graphics = TiledGraphics{tilemap = screenMap, color = {0.7, 0.5, 0.5}},
    collider = TiledCollider{tilemap = screenMap},
    position = {0, 0},
    scale = screenSize
}
game:addActor(screen)

-- Add colliders around the edge of the playing field
local leftWallTiles = TileMap
{
    index = singleTile,
    size = {1, screenSize[2] + 5}
}
leftWallTiles:setTiles(0, 0, 1, screenSize[2] + 5, 1)

local leftWall = Actor
{
    graphics = TiledGraphics{tilemap = leftWallTiles, color = {0.5, 0.5, 0.5}},
    collider = AabbCollider{}, -- no need for tiled collider
    position = {-1, -4},
    scale = {1, screenSize[2] + 5}
}
game:addActor(leftWall)

local rightWallTiles = TileMap
{
    index = singleTile,
    size = {5, screenSize[2] + 5}
}
rightWallTiles:setTiles(0, 0, 1, screenSize[2] + 5, 1)
rightWallTiles:setTiles(4, 0, 1, screenSize[2] + 5, 1)
rightWallTiles:setTiles(1, 4, 3, 1, 1)
rightWallTiles:setTiles(1, 9, 3, 1, 1)
rightWallTiles:setTiles(1, screenSize[2] + 4, 3, 1, 1)

local rightWall = Actor
{
    graphics = TiledGraphics{tilemap = rightWallTiles, color = {0.5, 0.5, 0.5}},
    collider = AabbCollider{},
    position = {screenSize[1], -4},
    scale = {5, screenSize[2] + 5}
}
game:addActor(rightWall)

local bottomWallTiles = TileMap
{
    index = singleTile,
    size = {screenSize[1], 1}
}
bottomWallTiles:setTiles(0, 0, screenSize[1], 1, 1)

local bottomWall = Actor
{
    graphics = TiledGraphics{tilemap = bottomWallTiles, color = {0.5, 0.5, 0.5}},
    collider = AabbCollider{},
    position = {0, screenSize[2]},
    scale = {screenSize[1], 1}
}
game:addActor(bottomWall)

-- set a fresh random seed when we start up
math.randomseed(os.time())

deck = {}
function getNext()
    if #deck == 0 then
        for i = 1, #tetrominoes do
            table.insert(deck, i)
            table.insert(deck, i)
        end
    end

    local i = math.random(1, #deck)
    local val = deck[i]
    table.remove(deck, i)
    return val
end

next = getNext()
defaultFallPeriod = 1
fallPeriod = defaultFallPeriod

function dropCurrent(down)
    if down then
        fallPeriod = 0.1
    else
        fallPeriod = defaultFallPeriod
    end
end

preview = Actor
{
    graphics = TiledGraphics{},
    members =
    {
        reset = function(self, val)
            local tetromino = tetrominoes[val]

            local graphics = self:getGraphics()
            graphics:setColor(tetromino.color)
            graphics:setTileMap(tetromino[1])

            local w, h = tetromino[1]:getSize()
            self:setScale(tetromino[1]:getSize())
            self:setPosition(screenSize[1]+1, 1)
        end
    }
}
game:addActor(preview)

-- Reuse the same Actor for all falling blocks
current = Actor
{
    graphics = TiledGraphics{},
    collider = TiledCollider{},
    members =
    {
        reset = function(self)
            self.num = next
            self.ori = 1
            self.dir = 0

            local tetromino = tetrominoes[next][1]

            local graphics = self:getGraphics()
            graphics:setColor(tetrominoes[next].color)
            graphics:setTileMap(tetromino)
            self:getCollider():setTileMap(tetromino)

            local w, h = tetromino:getSize()
            self:setScale(tetromino:getSize())
            self:setPosition(math.floor((screenSize[1] - w) / 2), -h)

            next = getNext()
            preview:reset(next)
        end,

        move = function(self, dir)
            self.dir = dir
        end,

        rotate = function(self, dir)
            local ori = self.ori + dir
            local nOri = #tetrominoes[self.num]
            if ori > nOri then
                ori = ori - nOri
            elseif ori < 1 then
                ori = ori + nOri
            end

            local tetromino = tetrominoes[self.num][ori]
            self:getGraphics():setTileMap(tetromino)
            self:getCollider():setTileMap(tetromino)
            --self:setScale(tetromino:getSize()) -- NOTE assuming all orientations share the same size

            -- If new orientation collides, reset to old orientation
            if self:testCollision(0, 0) then
                tetromino = tetrominoes[self.num][self.ori]
                self:getGraphics():setTileMap(tetromino)
                self:getCollider():setTileMap(tetromino)
                --self:setScale(tetromino:getSize())
                return
            end

            self.ori = ori
        end,

        update = function(self, time)
            local dx, dy = 0, 0
            local x, y = self:getPosition()
            local tetromino = self:getGraphics():getTileMap()
            local w, h = tetromino:getSize()

            if self.time then
                time = time + self.time
            end

            if time > fallPeriod then
                dy = 1
                time = math.fmod(time, fallPeriod)
            end

            self.time = time

            if self.dir < 0 then
                dx = -1
            elseif self.dir > 0 then
                dx = 1
            end
            self.dir = 0

            if self:testCollision(dx, 0) then
                dx = 0
            end

            if self:testCollision(dx, dy) then
                self:collided()
                return
            end

            self:setPosition(x + dx, y + dy)
        end,

        collided = function(self)--, hit)
            local x, y = self:getPosition()
            local tetromino = self:getGraphics():getTileMap()
            local w, h = tetromino:getSize()--self:getScale()?
            local sw, sh = screenSize[1], screenSize[2]
            --local yoff = (sh - h) - y

            -- copy current tetromino onto screen
            for yi = 0, w-1 do
                for xi = 0, w-1 do
                    local tile = tetromino:getTile(xi, yi)
                    if tile > 0 then
                        local sx, sy = xi + x, yi + y--yoff
                        if sx >= 0 and sx < sw and sy >= 0 and sy < sh then
                            screenMap:setTiles(sx, sy, 1, 1, tile)
                        end
                    end
                end
            end

            -- TODO add helper method to TileMap for scrolling/moving
            local shift = 0
            for yi = sh-1, 0, -1 do
                -- scan to check if row cleared (fully filled)
                local cleared = true
                for xi = 0, sw-1 do
                    if screenMap:getTile(xi, yi) < 1 then
                        cleared = false
                        break
                    end
                end
                -- skip line if cleared; otherwise shift line down
                if cleared then
                    -- TODO increment score
                    shift = shift + 1
                else
                    for xi = 0, sw-1 do
                        local tile = screenMap:getTile(xi, yi)
                        screenMap:setTiles(xi, yi+shift, 1, 1, tile)
                    end
                end
            end

            -- clear the top of the screen that was shifted down
            if shift > 0 then
                screenMap:setTiles(0, 0, sw, shift, 0)
            end

            self:reset()
        end
    }
}
game:addActor(current)
current:reset()

-- helper functions for key/gamepad events
function keyChanged(actor, method, arg)
    return function(down)
        method(actor, down, arg)
    end
end

function keyDown(actor, method, arg)
    return function(down)
        if down then method(actor, arg) end
    end
end

registerControl("up", keyDown(current, current.rotate, 1))
--registerControl("left", keyDown(rotateCurrent, 1))
--registerControl("right", keyDown(rotateCurrent, -1))
registerControl("left", keyDown(current, current.move, -1))
registerControl("right", keyDown(current, current.move, 1))
registerControl("down", dropCurrent)
