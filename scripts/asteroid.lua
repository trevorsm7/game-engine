setPortraitHint(true)
local screen_size = {12, 16}

local function spawnWall(canvas, position, scale)
    local wall = Actor
    {
        collider = AabbCollider{},
        transform = {position = position, scale = scale},
        members =
        {
            onCollide = function(self, hit)
                if hit.asteroid or hit.bolt then
                    local canvas = hit:getCanvas()
                    canvas:removeActor(hit)
                end
            end
        }
    }

    canvas:addActor(wall)
    return wall
end

local function spawnExplosion(canvas, pos, time)
    local N = 10
    local S = 2 * math.pi / N
    time = time or 0.5

    for i = 1, N do
        local vel_x = math.cos(i * S) * (math.random() * 3 + 5)
        local vel_y = math.sin(i * S) * (math.random() * 3 + 5)
        local particle = Actor
        {
            graphics = SpriteGraphics{sprite="round.tga", color={0.4, 0.4, 0.4}},
            physics = {velocity = {vel_x, vel_y}},
            transform = {position = pos, scale={0.25, 0.25}},
            members =
            {
                lifetime = time,

                onUpdate = function(self, delta)
                    local canvas = self:getCanvas()
                    self.lifetime = self.lifetime - delta
                    if self.lifetime < 0 then
                        canvas:removeActor(self)
                    end
                end
            }
        }
        canvas:addActor(particle)
    end
end

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
                local bolt = Actor
                {
                    graphics = SpriteGraphics{sprite="round.tga", color={0, 1, 0}},
                    collider = AabbCollider{},
                    physics = {mass = 1, velocity = {0, -bolt_vel}},
                    transform = {position = {my_x + 0.5, my_y - 1}, scale = {0.1, 1}},
                    members = {bolt = true}
                }
                canvas:addActor(bolt)
            end
        }
    }

    canvas:addActor(player)
    return player
end

local function spawnAsteroid(canvas)
    local sprite = "asteroid"..math.random(1,3)..".tga"
    local pos = math.random() * (screen_size[1] - 1)
    local vel = 4

    local asteroid = Actor
    {
        graphics = SpriteGraphics{sprite=sprite},
        collider = AabbCollider{},
        physics = {mass = math.huge, velocity = {0, vel}},
        transform = {position = {pos, 0}},
        members =
        {
            asteroid = true,

            onCollide = function(self, hit)
                local canvas = hit:getCanvas()

                if hit.bolt then
                    playSample("boom.wav")
                    spawnExplosion(canvas, {self:getPosition()})
                    canvas:removeActor(hit)
                    canvas:removeActor(self)
                    canvas:addScore(1)
                elseif hit.player then
                    hit.alive = false
                    playSample("boom.wav")
                    spawnExplosion(canvas, {self:getPosition()})
                    spawnExplosion(canvas, {hit:getPosition()})
                    canvas:removeActor(hit)
                    canvas:removeActor(self)
                end
            end
        }
    }

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
