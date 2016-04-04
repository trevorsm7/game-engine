game = Canvas.create({20, 20}, false)

tiles = Actor.create{tiles="tiles.map", collider=true}
game:addActor(tiles)
tiles:setPosition(-1, -1)
tiles:setScale(7, 7)
game:setCenter(2, 2)

--player = Actor.create{sprite="hero.tga", collider=true}
player = Actor.create{tiles="heropad.map", collider=true}
game:addActor(player)
player:setPosition(-2, -2)
player:setScale(3, 3)

-- generate a callback function to move the player in a direction
function player:keyDown(method, arg)
    return function(down)
        if down then method(self, arg) end
    end
end

function player:move(dir)
    local canvas = self:getCanvas()
    if not canvas then return end

    local x, y = self:getPosition()
    x = x + dir[1] * .5; y = y + dir[2] * .5
    --local hit = canvas:getCollision(x + 0.5, y + 0.5)
    --local hit = canvas:getCollision(x, y)

    if not hit then
        self:setPosition(x, y)
    end
end

function player:update(delta)
    if self:testCollision(0, 0) then
        self:setColor{1, .5, .5}
    else
        self:setColor{1, 1, 1}
    end
end

registerControl("left", player:keyDown(player.move, {-1, 0}))
registerControl("right", player:keyDown(player.move, {1, 0}))
registerControl("down", player:keyDown(player.move, {0, -1}))
registerControl("up", player:keyDown(player.move, {0, 1}))

nerd = Actor.create{sprite="nerd.tga", collider=true}
game:addActor(nerd)
nerd:setPosition(4, 4)
