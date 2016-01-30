local white = {1, 1, 1}
local grey = {0.6, 0.6, 0.6}
local rouge = {1, 0, 0}
local green = {0, 1, 0}
local blue = {0, 0, 1}
local yellow = {1, 1, 0}

-- use a global timer for managing turns
local gameTime = 0

local function newPlayer(canvas, x, y)
    local player = Actor.create(rouge, true)
    canvas:addActor(player)
    player:setPosition(x, y)
    player.player = true
    player.stepTime = 1

    -- generate a callback function to move the player in a direction
    function player:moveFactory(dir)
        return function(down)
            if down then self:move(dir) end
        end
    end

    function player:move(dir)
        local canvas = self:getCanvas()
        if not canvas then return end

        local x, y = self:getPosition()
        x = x + dir[1]; y = y + dir[2]
        local hit = canvas:getCollision(x + 0.5, y + 0.5)

        if not hit then
            self:setPosition(x, y)
            gameTime = gameTime + self.stepTime
        elseif hit.enemy then
            hit:attack(self)
            gameTime = gameTime + self.stepTime
        elseif hit.interact then
            gameTime = gameTime + hit:interact(self)
        end
    end

    return player
end

local function newNerd(canvas, x, y)
    local nerd = Actor.create(yellow, true)
    canvas:addActor(nerd)
    nerd:setPosition(x, y)
    nerd.enemy = true
    nerd.stepTime = 1
    nerd.time = gameTime

    local function sign(n)
        if n > 0 then
            return 1
        elseif n < 0 then
            return -1
        end
        return 0
    end

    function nerd:update(delta)
        local canvas = self:getCanvas()
        if not canvas then return end
        if not player or not player:getCanvas() then return end

        -- instead of looping, we could just process one action per update
        -- this would break if the player spammed expensive actions
        while true do
            local timeLeft = gameTime - self.time

            local x, y = self:getPosition()
            local px, py = player:getPosition()
            local dx = px - x; local dy = py - y

            local horzOpen = not canvas:getCollision(x + sign(dx), y)
            local vertOpen = not canvas:getCollision(x, y + sign(dy))

            -- no intelligent pathfinding for the moment
            if self.stepTime <= timeLeft then
                if ((math.abs(dx) == 1 and dy == 0) or (math.abs(dy) == 0 and dx == 0)) then
                    self.time = self.time + self.stepTime
                    self:attack(player)
                elseif (math.abs(dx) >= math.abs(dy) or not vertOpen) and horzOpen then
                    self.time = self.time + self.stepTime
                    self:setPosition(x + sign(dx), y)
                elseif vertOpen then
                    self.time = self.time + self.stepTime
                    self:setPosition(x, y + sign(dy))
                else
                    -- no moves available; perform a wait to drain time
                    self.time = self.time + self.stepTime
                    --self.time = gameTime
                    break
                end
            else
                -- not enough time to take an action
                break
            end
        end
    end

    return nerd
end

local function newWall(canvas, x, y, w, h)
    local wall = Actor.create(white, true)
    canvas:addActor(wall)
    wall:setPosition(x, y)
    wall:setScale(w, h)

    return wall
end

local function newDoor(canvas, x, y)
    local door = Actor.create(grey, true)
    canvas:addActor(door)
    door:setPosition(x, y)
    door.open = false

    function door:interact(actor)
        local canvas = self:getCanvas()
        if canvas then
            if self.open then
                self:setScale(1, 1)
                self.open = false
                return 1 -- time to close door
            else
                --canvas:removeActor(self)
                self:setScale(0.1, 1)
                self.open = true
                return 1 -- time to open door
            end
        end
        return 0
    end

    return door
end

-- set a fresh random seed when we start up
math.randomseed(os.time())

game = Canvas.create{0, 0, 0, 0}

player = newPlayer(game, 1, 1)
registerKey("key_left", player:moveFactory{-1, 0})
registerKey("key_right", player:moveFactory{1, 0})
registerKey("key_down", player:moveFactory{0, -1})
registerKey("key_up", player:moveFactory{0, 1})

newWall(game, 0, 0, 6, 1)
newWall(game, 0, 5, 6, 1)
newWall(game, 0, 1, 1, 4)
newWall(game, 5, 1, 1, 3)
newDoor(game, 5, 4)

newNerd(game, 10, 10)
