red = {1, 0, 0}
white = {1, 1, 1}

game = Canvas{size = {20, 20}, fixed = false}
game:setCenter(0.5, -5)
addCanvas(game)

player = Actor
{
    graphics = SpriteGraphics{sprite="hero.tga"},
    collider = AabbCollider{},
    physics = {cor=0.01, cof=0.2},
    transform = {position = {-4, 0}}
}
game:addActor(player)
player.velX = 0

function player:onCollide(hit)
    hit:getGraphics():setColor(red)
end

function player:onUpdate(delta)
    self:addAcceleration(0, 10)

    local velX, velY = self:getVelocity()
    if self:testCollision(0, .01) then
        self:setVelocity(self.velX, velY)
    elseif self.velX ~= 0 then
        self:setVelocity(self.velX / 2, velY)
    end
end

allBoxes = {}

function addBox(x, y)
    local num = #allBoxes + 1
    local box = Actor
    {
        graphics = SpriteGraphics{sprite="square.tga"},
        collider = AabbCollider{},
        transform = {position = {x, y}}
    }
    game:addActor(box)
    allBoxes[num] = box
    return box
end

function updateBox(self, delta)
    self:addAcceleration(0, 10)
end

function addMovingBox(x, y)
    local num = #allBoxes + 1
    local box = Actor
    {
        graphics = SpriteGraphics{sprite="square.tga"},
        collider = AabbCollider{},
        physics = {cor=0.8, cof=0.2},
        transform = {position = {x, y}}
    }
    game:addActor(box)
    box.onUpdate = updateBox
    allBoxes[num] = box
    return box
end

--for col = -3, 3 do
do
    local col = 0
    for row = 2, 5 do
        addMovingBox(col, -row)
    end
end

addBox(-5, 1)
addBox(-4, 1)
addBox(-3, 1)
addBox(-2, 1)
addBox(-1, 1)
addBox(0, 1)
addBox(1, 1)
addBox(2, 1)
addBox(3, 1)
addBox(4, 1)
addBox(5, 1)

addBox(-5, 0)
addBox(-5, -1)
addBox(-5, -2)
addBox(5, -2)
addBox(5, -1)
addBox(5, 0)

--addBox(0, -4)
--addBox(0, -5)
--addBox(0, -6)

addBox(-3, -8)
addBox(-3, -9)
addBox(-3, -10)

addBox(3, -10)
addBox(3, -9)
addBox(3, -8)

function resetBoxes()
    for i, box in ipairs(allBoxes) do
        box:getGraphics():setColor(white)
    end
end

function jump(velY)
    -- TODO PlatformCollider
    -- NOTE switched signs so you kick away from the wall
    local wall = player.velX > 0 and -.1 or player.velX < 0 and .1 or 0
    if player:testCollision(wall, .01) then
        resetBoxes()
        local velX = player:getVelocity()
        player:setVelocity(velX, velY)
    end
end

function moveBox(down, velX)
    if down then
        player.velX = velX
    elseif velX == player.velX then
        player.velX = 0
    end
end

function warpBox(dir)
    resetBoxes()
    local x, y = player:getPosition()
    x = x + dir[1]
    y = y + dir[2]
    player:setPosition(x, y)
end

function keyAction(method, arg)
    return function(down)
        method(down, arg)
    end
end

-- generate a callback function to move in a given direction
function keyDown(method, arg)
    return function(down)
        if down then method(arg) end
    end
end

-- IDEA if we continue to use registerControl this way, have it take a 3rd arg to pass to the callback
registerControl("right", keyAction(moveBox, 5))
registerControl("left", keyAction(moveBox, -5))
registerControl("action", keyDown(jump, -8))
registerControl("up", keyDown(jump, -8))
registerControl("down", keyDown(warpBox, {0, 0.1}))
