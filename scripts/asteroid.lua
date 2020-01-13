setPortraitHint(true)
local screen_size = {12, 16}

local function createPool(size, template)
    local pool = {}
    template._pool = pool
    pool._template = template

    for i = 1, size do
        pool[i] = Actor(template)
    end

    pool.pull = function(self)
        local object = table.remove(self)
        if not object then
            -- clone new instance if pool is empty
            object = Actor(self._template)
        end
        return object
    end

    pool.push = function(self, object)
        table.insert(self, object)
    end

    return pool
end

local function spawnWall(canvas, position, scale)
    local wall = Actor
    {
        collider = AabbCollider{},
        transform = {position = position, scale = scale},
        members =
        {
            onCollide = function(self, hit)
                if hit.asteroid or hit.bolt then
                    hit:kill()
                end
            end
        }
    }

    canvas:addActor(wall)
    return wall
end

local explosionPool = createPool(20, Actor {
    graphics = SpriteGraphics{sprite="round.tga", color={0.4, 0.4, 0.4}},
    physics = {},
    transform = {scale={0.25, 0.25}},
    members = {
        onUpdate = function(self, delta)
            self.lifetime = self.lifetime - delta
            if self.lifetime < 0 then
                self:kill()
            end
        end,

        kill = function(self)
            self:getCanvas():removeActor(self)
            self._pool:push(self)
        end
    }
})

local function spawnExplosion(canvas, size, pos, time)
    local S = 2 * math.pi / size
    time = time or 0.5

    for i = 1, size do
        local vel_x = math.cos(i * S) * (math.random() * 3 + 5)
        local vel_y = math.sin(i * S) * (math.random() * 3 + 5)
        local particle = explosionPool:pull()
        particle.lifetime = time
        particle:setVelocity(vel_x, vel_y)
        particle:setPosition(pos[1], pos[2])
        canvas:addActor(particle)
    end
end

local boltPool = createPool(10, Actor {
    graphics = SpriteGraphics{sprite="round.tga", color={0, 1, 0}},
    collider = AabbCollider{},
    physics = {mass = 1},
    transform = {scale = {0.1, 1}},
    members = {
        bolt = true,

        kill = function(self)
            self:getCanvas():removeActor(self)
            self._pool:push(self)
        end
    }
})

local function spawnPlayer(canvas)
    local x = (screen_size[1] - 1) / 2
    local y = screen_size[2] - 2
    local ship_vel = 6
    local ship_acc = 5
    local bolt_vel = 10

    local player = Actor
    {
        graphics = SpriteGraphics{sprite="ship.tga"},
        collider = AabbCollider{},
        physics = {mass = math.huge},
        transform = {position = {x, y}},
        members =
        {
            player = true,
            alive = true,

            vel_x = 0,
            vel_y = 0,
            firing = false,
            cooldown = 0,

            max_cooldown = 0.2,

            onUpdate = function(self, delta)
                local vel_x, vel_y = self:getVelocity()
                local acc_x = (self.vel_x - vel_x) * ship_acc
                local acc_y = (self.vel_y - vel_y) * ship_acc
                self:addAcceleration(acc_x, acc_y)

                self.cooldown = self.cooldown - delta
                if self.firing and self.cooldown < 0 then
                    self.cooldown = self.max_cooldown
                    self:spawnBolt()
                end
            end,

            move = function(self, down, axis, dir)
                local vel = dir * ship_vel
                if down then
                    self[axis] = vel
                elseif self[axis] == vel then
                    self[axis] = 0
                end
            end,

            fire = function(self, down)
                self.firing = down
            end,

            spawnBolt = function(self)
                local canvas = self:getCanvas()

                playSample("laser.wav")
                local my_x, my_y = self:getPosition()
                local bolt = boltPool:pull()
                bolt:setPosition(my_x + 0.5, my_y - 1)
                bolt:setVelocity(0, -bolt_vel)
                canvas:addActor(bolt)
            end,

            kill = function(self)
                self.alive = false
                self:getCanvas():removeActor(self)
            end
        }
    }

    canvas:addActor(player)
    return player
end

local asteroidPool = createPool(10, Actor {
    collider = AabbCollider{},
    physics = {mass = math.huge},
    members =
    {
        asteroid = true,

        onCollide = function(self, hit)
            local canvas = hit:getCanvas()

            if hit.bolt then
                playSample("boom.wav")
                spawnExplosion(canvas, 10, {self:getPosition()})
                hit:kill()
                self:kill()
                canvas:addScore(1)
            elseif hit.player then
                playSample("boom.wav")
                spawnExplosion(canvas, 10, {self:getPosition()})
                spawnExplosion(canvas, 10, {hit:getPosition()})
                hit:kill()
                self:kill()
            end
        end,

        kill = function(self)
            self:getCanvas():removeActor(self)
            self._pool:push(self)
        end
    }
})

local asteroidSprites = {
    SpriteGraphics{sprite="asteroid1.tga"},
    SpriteGraphics{sprite="asteroid2.tga"},
    SpriteGraphics{sprite="asteroid3.tga"},
}

local function spawnAsteroid(canvas)
    local sprite = asteroidSprites[math.random(1, #asteroidSprites)]
    local pos = math.random() * (screen_size[1] - 1)
    local vel = 4

    local asteroid = asteroidPool:pull()
    asteroid:setGraphics(sprite)
    asteroid:setPosition(pos, 0)
    asteroid:setVelocity(0, vel)

    canvas:addActor(asteroid)
    return asteroid
end

local function createText(canvas, pos, size, scale, centered)
    x = pos[1]
    y = pos[2]
    if centered then
        x = (x - size[1] * scale[1]) / 2
        y = (y - size[2] * scale[2]) / 2
    end
    local text = Actor
    {
        graphics = TiledGraphics
        {
            tilemap = TileMap
            {
                tileset = TileSet
                {
                    filename = "gnsh-green.tga",
                    size = {20, 5},
                },
                size = size
            }
        },
        transform = {position = {x, y}, scale = scale or {1, 1}},
        members =
        {
            tilemap = fontTiles,

            setTextL = function(self, line, text)
                local tilemap = self:getGraphics():getTileMap()
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
                local tilemap = self:getGraphics():getTileMap()
                local size, _ = tilemap:getSize()
                local chars = math.min(string.len(text), size)
                local offset = math.max(size-chars - 1, -1)
                for i = 1, chars do
                    tilemap:setTiles(i+offset, line, 1, 1, string.byte(text, i) - 31)
                end
                if chars < size then
                    tilemap:setTiles(0, line, size-chars, 1, 0)
                end
            end
        }
    }

    canvas:addActor(text)
    return text
end

local overlay = Canvas
{
    camera = Camera2D
    {
        size = screen_size,
        fixed = true
    }
}

local scoreText = createText(overlay, {0, 0}, {10, 1}, {0.5, 1})
local respawnText = createText(overlay, screen_size, {9, 1}, {0.5, 1}, true)

local game = Canvas
{
    camera = Camera2D
    {
        size = screen_size,
        fixed = true
    },
    members =
    {
        score = 0,

        asteroid_cooldown = 1,
        max_asteroid_cooldown = 0.5,
        spawning_asteroids = true,

        player_cooldown = 0,
        max_player_cooldown = 4,

        onUpdatePost = function(self, delta)
            if not player or not player.alive then
                -- wait for player cooldown to respawn
                self.asteroid_cooldown = 1
                self.player_cooldown = self.player_cooldown - delta
                respawnText:getGraphics():setVisible(true)
                respawnText:setTextL(0, "Respawn:"..tostring(math.floor(self.player_cooldown)))
                if self.player_cooldown < 0 then
                    player = spawnPlayer(self)
                    respawnText:getGraphics():setVisible(false)
                    self.player_cooldown = self.max_player_cooldown
                end
            else
                -- wait for asteroid cooldown
                self.asteroid_cooldown = self.asteroid_cooldown - delta
                if self.spawning_asteroids and self.asteroid_cooldown < 0 then
                    self.asteroid_cooldown = self.max_asteroid_cooldown
                    spawnAsteroid(self)
                end
            end
        end,

        addScore = function(self, score)
            self.score = self.score + score
            scoreText:setTextL(0, "Score:"..tostring(self.score))
        end
    }
}

game:addScore(0)
spawnWall(game, {-1, 0}, {1, screen_size[2]})
spawnWall(game, {screen_size[1], 0}, {1, screen_size[2]})
spawnWall(game, {0, -1}, {screen_size[1], 1})
spawnWall(game, {0, screen_size[2]}, {screen_size[1], 1})

addCanvas(game)
addCanvas(overlay)

local function playerMove(axis, vel)
    return function(down)
        if player then
            player:move(down, axis, vel)
        end
    end
end

local function playerFire()
    return function(down)
        if player then
            player:fire(down)
        end
    end
end

registerControl("left", playerMove("vel_x", -1))
registerControl("right", playerMove("vel_x", 1))
registerControl("down", playerMove("vel_y", 1))
registerControl("up", playerMove("vel_y", -1))
registerControl("action", playerFire())
