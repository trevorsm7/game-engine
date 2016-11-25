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

local game = Canvas
{
    size = {screenSize[1] + 6, screenSize[2] + 1},
    fixed = true
}
game:setOrigin(-1, 0)
addCanvas(game)

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
    collider = TiledCollider{tilemap = screenMap}
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
    transform = {position = {-1, -4}}
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
    transform = {position = {screenSize[1], -4}}
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
    transform = {position = {0, screenSize[2]}}
}
game:addActor(bottomWall)

local fontTiles = TileMap
{
    index = TileIndex
    {
        sprite = "gnsh-green.tga",
        size = {20, 5},
    },
    size = {7, 4}
}

scoreText = Actor
{
    graphics = TiledGraphics{tilemap = fontTiles},
    transform = {position = {screenSize[1] + 1, 6}, scale = {3/7, 1}},
    members =
    {
        tilemap = fontTiles,

        setTextL = function(self, line, text)
            local tilemap = self.tilemap
            local size, _ = tilemap:getSize()
            local chars = math.min(string.len(text), size)
            for i = 1, chars do
                tilemap:setTiles(i-1, line, 1, 1, string.byte(text, i) - 31)
            end
            if chars < size then
                tilemap:setTiles(chars, line, size-chars, 1, 0)
            end
        end,

        setTextR = function(self, line, text)
            local tilemap = self.tilemap
            local size, _ = tilemap:getSize()
            local chars = math.min(string.len(text), size)
            local offset = math.max(size-chars - 1, -1)
            for i = 1, chars do
                tilemap:setTiles(i+offset, line, 1, 1, string.byte(text, i) - 31)
            end
            if chars < size then
                tilemap:setTiles(0, line, size-chars, 1, 0)
            end
        end,

        setLevel = function(self, level)
            local text = tostring(level)
            self:setTextR(1, text)
        end,

        setScore = function(self, score)
            local text = tostring(score)
            self:setTextR(3, text)
        end
    }
}
game:addActor(scoreText)

count = 0
level = 1
scoreText:setTextL(0, "Level:")
scoreText:setLevel(level)

score = 0
scoreText:setTextL(2, "Score:")
scoreText:setScore(score)

local deck = {}
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

nextTetro = getNext()
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
            self.num = nextTetro
            self.ori = 1
            self.dir = 0

            local tetromino = tetrominoes[nextTetro][1]

            local graphics = self:getGraphics()
            graphics:setColor(tetrominoes[nextTetro].color)
            graphics:setTileMap(tetromino)
            self:getCollider():setTileMap(tetromino)

            local w, h = tetromino:getSize()
            self:setPosition(math.floor((screenSize[1] - w) / 2), -h)

            nextTetro = getNext()
            preview:reset(nextTetro)
        end,

        move = function(self, dir)
            self.dir = dir
        end,

        rotate = function(self, dir)
            if not self:getCanvas() then
                return
            end

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

            -- If new orientation collides, reset to old orientation
            if self:testCollision(0, 0) then
                tetromino = tetrominoes[self.num][self.ori]
                self:getGraphics():setTileMap(tetromino)
                self:getCollider():setTileMap(tetromino)
                return
            end

            self.ori = ori
        end,

        onUpdate = function(self, time)
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

            -- copy current tetromino onto screen
            for yi = 0, w-1 do
                for xi = 0, w-1 do
                    local tile = tetromino:getTile(xi, yi)
                    if tile > 0 then
                        local sx, sy = xi + x, yi + y
                        if sx >= 0 and sx < sw and sy >= 0 and sy < sh then
                            screenMap:setTiles(sx, sy, 1, 1, tile)
                        end
                    end
                end
            end

            if y < 0 then
                scoreText:setTextL(0, "Game")
                scoreText:setTextR(1, "Over")
                local canvas = self:getCanvas()
                if canvas then canvas:removeActor(self) end
                return
            end

            -- check for cleared rows along tetromino outline
            local shift = 0
            local bottom = math.min(y+h, sh) - 1
            for yi = bottom, y, -1 do
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
                    shift = shift + 1
                elseif shift > 0 then
                    screenMap:moveTiles(0, yi, sw, 1, 0, shift)
                end
            end

            -- shift remainder of the screen and clear the top
            if shift > 0 then
                screenMap:moveTiles(0, 0, sw, y, 0, shift)
                screenMap:setTiles(0, 0, sw, shift, 0)

                -- add 10, 30, 60, 100 points depending on rows cleared
                score = score + shift*(shift+1)*level
            end

            score = score + level
            scoreText:setScore(score)

            count = count + 1
            if count == 10 then
                level = level + 1
                defaultFallPeriod = math.max(defaultFallPeriod * 0.8, 0.1)
                count = 0
                scoreText:setLevel(level)
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

function saveGame(down)
    if down then saveState() end
end

registerControl("up", keyDown(current, current.rotate, 1))
--registerControl("left", keyDown(rotateCurrent, 1))
--registerControl("right", keyDown(rotateCurrent, -1))
registerControl("left", keyDown(current, current.move, -1))
registerControl("right", keyDown(current, current.move, 1))
registerControl("down", dropCurrent)
registerControl("action", saveGame)
