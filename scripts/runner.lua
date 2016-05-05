game = Canvas({20, 20}, false)

player = Actor{sprite="hero.tga", collider=true, physics=true, cor=0.01, cof=0}
game:addActor(player)
player:setPosition(2, 0)
game:setCenter(10, 0)
camY = 0
--game:setCenter(player)
function player:update(delta)
    local canvas = self:getCanvas()
    if not canvas then return end

    local x, y = self:getPosition()
    if x < 0 or y < -10 then
        print("u is ded")
        canvas:removeActor(self)
    end

    --canvas:setCenter(self)
    camY = camY + (y - camY) * delta
    canvas:setCenter(10, camY)

    self:addAcceleration(0, -10)
end
function player:jump()
    --TODO reset jump counter when we hit top of a platform instead
    if self:testCollision(0, -.01) then
        self.canDoubleJump = true
        local velX = self:getVelocity()
        self:setVelocity(velX, 8)
    elseif self.canDoubleJump then
        self.canDoubleJump = false
        local velX = self:getVelocity()
        self:setVelocity(velX, 8)
    end
end

function addPlatform(canvas, x, y, w, h)
    local platform = Actor{sprite="square.tga", collider=true, physics=true, mass=math.huge}
    canvas:addActor(platform)
    platform:setPosition(x, y)
    platform:setScale(w, h)
    platform:setVelocity(-5, 0)
    function platform:update()
        self:addAcceleration(-0.2, 0)
        local x, y = self:getPosition()
        if x < -10 then
            self:setPosition(x + 30, y)
        end
    end
    return box
end

addPlatform(game, 0, -2, 10, 1)
addPlatform(game, 15, 0, 10, 1)


function jump(actor)
    actor:jump()
end

function keyAction(method, arg)
    return function(down)
        method(down, arg)
    end
end

function keyDown(method, arg)
    return function(down)
        if down then method(arg) end
    end
end

registerControl("action", keyDown(jump, player))
