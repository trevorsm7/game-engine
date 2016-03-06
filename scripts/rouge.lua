local white = {1, 1, 1}
local grey = {0.6, 0.6, 0.6}
local darkGrey = {0.3, 0.3, 0.3}
local rouge = {1, 0, 0}
local green = {0, 1, 0}
local blue = {0, 0, 1}
local yellow = {1, 1, 0}

-- use a global timer for managing turns
local gameTime = 0

local function newPlayer(canvas, x, y)
    local player = Actor.create{sprite="hero.tga", collider=true}
    canvas:addActor(player)
    player:setPosition(x, y)
    canvas:setCenter(x + 0.5, y + 0.5)
    player.player = true
    player.stepTime = 1

    function player:update(delta)
        local canvas = self:getCanvas()
        if canvas then
            local x, y = self:getPosition()
            canvas:setCenter(x + 0.5, y + 0.5)
            --canvas:setVisible(true) -- hack to hide flicker in first frame
        end
    end

    -- generate a callback function to move the player in a direction
    function player:keyDown(method, arg)
        return function(down)
            if down then method(self, arg) end
        end
    end

    function player:idle()
        local canvas = self:getCanvas()
        if not canvas then return end

        gameTime = gameTime + self.stepTime
    end

    function player:attack(target)
        print("Player attacks!")
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
    end

    return player
end

local function newNerd(canvas, x, y)
    local nerd = Actor.create{sprite="nerd.tga", collider=true}
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

    function nerd:attack(target)
        print("Nerd attacks!")
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

            --local horzOpen = not canvas:getCollision(x + sign(dx) + 0.5, y + 0.5)
            --local vertOpen = not canvas:getCollision(x + 0.5, y + sign(dy) + 0.5)
            local horzOpen = not canvas:getCollision(x + sign(dx), y)
            local vertOpen = not canvas:getCollision(x, y + sign(dy))

            -- no intelligent pathfinding for the moment
            if self.stepTime <= timeLeft then
                if ((math.abs(dx) == 1 and dy == 0) or (math.abs(dy) == 1 and dx == 0)) then
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
    local wall = Actor.create{sprite="square.tga", collider=true}
    canvas:addActor(wall)
    wall:setPosition(x, y)
    wall:setScale(w, h)

    return wall
end

local function newFloor(canvas, x, y, w, h)
    local floor = Actor.create{sprite="square.tga", layer=-1, color=darkGrey}
    canvas:addActor(floor)
    floor:setPosition(x, y)
    floor:setScale(w, h)

    return floor
end

local function newDoor(canvas, x, y)
    local door = Actor.create{sprite="door.tga", collider=true}
    canvas:addActor(door)
    door:setPosition(x, y)
    door.open = false

    function door:interact(actor)
        local canvas = self:getCanvas()
        if canvas then
            if self.open then
                self:setScale(1, 1)
                self:setCollidable(true)
                self.open = false
                return 1 -- time to close door
            else
                self:setScale(0.1, 1)
                self:setCollidable(false)
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

game = Canvas.create({20, 20}, false)
--game:setVisible(false) -- hack to hide flicker in first frame

player = newPlayer(game, 1, 1)
registerControl("left", player:keyDown(player.move, {-1, 0}))
registerControl("right", player:keyDown(player.move, {1, 0}))
registerControl("down", player:keyDown(player.move, {0, -1}))
registerControl("up", player:keyDown(player.move, {0, 1}))
registerControl("action", player:keyDown(player.idle))

newFloor(game, 1, 1, 4, 4)
newWall(game, 0, 0, 6, 1)
newWall(game, 0, 5, 6, 1)
newWall(game, 0, 1, 1, 4)
newWall(game, 5, 1, 1, 3)
newDoor(game, 5, 4)

newNerd(game, 10, 10)
