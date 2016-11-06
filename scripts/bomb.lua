local white = {1, 1, 1}
local grey = {0.6, 0.6, 0.6}
local red = {1, 0, 0}
local green = {0, 1, 0}
local blue = {0, 0, 1}
local yellow = {1, 1, 0}

-- get a random position guranteed to be empty
local function getEmptySpace(canvas)
    local x, y
    repeat
        x = math.random(1, 18)
        y = math.random(1, 13)
    until not canvas:getCollision(x + 0.5, y + 0.5)
    return x, y
end

local function newFire(canvas, x, y)
    local fire = Actor{graphics=SpriteGraphics{color=yellow, sprite="round.tga"}}
    canvas:addActor(fire)
    fire:setPosition(x, y)
    fire.time = 1

    function fire:onUpdate(delta)
        local canvas = self:getCanvas()
        if not canvas then return end

        self.time = self.time - delta

        if self.time > 0 then
            local x, y = self:getPosition()
            local hit = canvas:getCollision(x+0.5, y+0.5)
            if hit and hit.burn then
                hit:burn()
            end
            return
        end

        -- TODO: should remove from current canvas rather than from original
        canvas:removeActor(self)
    end
end

local function newBomb(canvas, x, y)
    local bomb = Actor{graphics=SpriteGraphics{color=red, sprite="round.tga"}, collider=AabbCollider{}}
    canvas:addActor(bomb)
    bomb:setPosition(x, y)
    bomb.time = 2

    function bomb:onUpdate(delta)
        local canvas = self:getCanvas()
        if not canvas then return end

        self.time = self.time - delta

        if self.time <= 0 then
            local x, y = self:getPosition()

            -- remove self from Canvas and replace with fire
            canvas:removeActor(self)
            newFire(canvas, x, y)

            local function explode(x, y)
                local hit = canvas:getCollision(x+0.5, y+0.5)
                if hit and hit.burn and not hit:burn() then
                    return hit -- don't spawn fire on non-burnable tiles
                end
                newFire(canvas, x, y)
                return hit
            end

            -- explode in each direction, stopping if we hit something
            for dx = 1, 3 do
                if explode(x + dx, y) then
                    break
                end
            end

            for dx = 1, 3 do
                if explode(x - dx, y) then
                    break
                end
            end

            for dy = 1, 3 do
                if explode(x, y + dy) then
                    break
                end
            end

            for dy = 1, 3 do
                if explode(x, y - dy) then
                    break
                end
            end
        end
    end
end

local function newWall(canvas, x, y)
    local wall = Actor{graphics=SpriteGraphics{color=white, sprite="square.tga"}, collider=AabbCollider{}}
    canvas:addActor(wall)
    wall:setPosition(x, y)

    function wall:burn()
        return false
    end
end

local function newBreakable(canvas, x, y)
    local breakable = Actor{graphics=SpriteGraphics{color=grey, sprite="square.tga"}, collider=AabbCollider{}}
    canvas:addActor(breakable)
    breakable:setPosition(x, y)

    function breakable:burn()
        local canvas = self:getCanvas()
        if not canvas then return false end

        canvas:removeActor(self)
        return true
    end
end

local function newNerd(canvas)--, x, y)
    local x, y = getEmptySpace(canvas)
    local nerd = Actor{graphics=SpriteGraphics{color=red, sprite="square.tga"}, collider=AabbCollider{}}
    canvas:addActor(nerd)
    nerd:setPosition(x, y)
    nerd.dirList = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}
    nerd.dir = math.random(1, 4)
    nerd.moveWait = 0

    function nerd:burn()
        local canvas = self:getCanvas()
        if not canvas then return false end

        print("Aieee!")
        self.dead = true
        canvas:removeActor(self)
        return true
    end

    function nerd:onUpdate(delta)
        local canvas = self:getCanvas()
        if not canvas then return end

        if self.moveWait > delta then
            self.moveWait = self.moveWait - delta
            return
        end

        local nx, ny = self:getPosition()
        nx = nx + nerd.dirList[nerd.dir][1]
        ny = ny + nerd.dirList[nerd.dir][2]
        hit = canvas:getCollision(nx + 0.5, ny + 0.5)

        if hit and hit.player and hit.burn then
            hit:burn()
        end

        if math.random() < 0.1 or hit then
            nerd.dir = math.random(1, 4)
        else
            self:setPosition(nx, ny)
        end

        self.moveWait = 0.2
    end

    return nerd
end

local function newPlayer(canvas, x, y)
    local player = Actor{graphics=SpriteGraphics{color=blue, sprite="square.tga"}, collider=AabbCollider{}}
    canvas:addActor(player)
    player:setPosition(x, y)
    player.player = true
    player.dirList = {}
    player.moveWait = 0

    function player:burn()
        local canvas = self:getCanvas()
        if not canvas then return false end

        print("Ouch!")
        self.dead = true
        canvas:removeActor(self)
        return true
    end

    -- generate a callback function to move the snake in a direction
    function player:moveFactory(dir)
        return function(down)
            if down then
                table.insert(self.dirList, 1, dir)
            else
                for i, v in ipairs(self.dirList) do
                    if v[1] == dir[1] and v[2] == dir[2] then
                        table.remove(self.dirList, i)
                        break
                    end
                end
            end
            if not self.dead then
                self:move()
            end
        end
    end

    function player:move()
        local canvas = self:getCanvas()
        if not canvas then return end

        if self.moveWait <= 0 then
            local x, y = self:getPosition()
            local dir = self.dirList[1] or {0, 0}
            x = x + dir[1]
            y = y + dir[2]
            if not canvas:getCollision(x, y) then
                self:setPosition(x, y)
                self.moveWait = 0.2
            end
        end
    end

    function player:onUpdate(delta)
        if self.moveWait > 0 then
            self.moveWait = self.moveWait - delta
        end
        self:move()
    end

    function player:bombFactory()
        return function(down)
            if down then
                if not self.dead then
                    local x, y = self:getPosition()
                    newBomb(canvas, x, y)
                else
                    self.dead = false
                    canvas:addActor(self)
                    self:setPosition(x, y) -- move back to starting pos
                end
            end
        end
    end

    return player
end

local game = Canvas({20, 15}, true)
addCanvas(game)

local player = newPlayer(game, 1, 1)
registerControl("left", player:moveFactory{-1, 0})
registerControl("right", player:moveFactory{1, 0})
registerControl("down", player:moveFactory{0, 1})
registerControl("up", player:moveFactory{0, -1})
registerControl("action", player:bombFactory())

for x = 0, 19 do
    newWall(game, x, 0)
    newWall(game, x, 14)
end
for y = 1, 13 do
    newWall(game, 0, y)
    newWall(game, 19, y)
end
for x = 2, 17, 3 do
    for y = 2, 12, 2 do
        newWall(game, x, y)
    end
end
for i = 1, 20 do
    newBreakable(game, getEmptySpace(game))
end
for i = 1, 5 do
    newNerd(game)
end
