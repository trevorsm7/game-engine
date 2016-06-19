-- create a 20x15 game window
game = Canvas({20, 15}, true)

-- put invisible walls around the edges of the screen
leftWall = Actor
{
    collider = AabbCollider{group = 4, mask = 3},
    position = {-1, -1},
    scale = {1, 16}
}
game:addActor(leftWall)

rightWall = Actor
{
    collider = AabbCollider{group = 4, mask = 3},
    position = {20, -1},
    scale = {1, 16}
}
game:addActor(rightWall)

topWall = Actor
{
    collider = AabbCollider{group = 4, mask = 1},
    position = {0, 15},
    scale = {20, 1}
}
game:addActor(topWall)

bottomWall = Actor
{
    collider = AabbCollider{group = 4, mask = 1},
    position = {0, -2},
    scale = {20, 1}
}
game:addActor(bottomWall)

-- reset the ball if it hits the bottom wall
function bottomWall:collided(hit)
    if hit == ball then
        ball.attached = true
        ball:update()
    end
end

-- create the paddle
-- TODO can we have COR=0 between paddle-walls while still having COR=1 between paddle-ball and ball-walls?
-- TODO make paddle a top edge only collider
paddle = Actor
{
    --graphics = SpriteGraphics{sprite = "square.tga"},
    --collider = AabbCollider{group = 2, mask = 5},
    physics = {mass = math.huge},
    scale = {4, 1}
}
paddle:setGraphics(SpriteGraphics{sprite = "square.tga"})
paddle:setCollider(AabbCollider{group = 2, mask = 5})
game:addActor(paddle)

paddle.vel = 0
function paddle:update()
    velx = self:getVelocity()
    self:addAcceleration((self.vel - velx) * 5, 0)
end

-- create the ball
ball = Actor
{
    graphics=SpriteGraphics{sprite="round.tga"},
    collider=AabbCollider{group=1},
    physics={} --cof=1
}
game:addActor(ball)

-- keep the ball stuck to the paddle until it is launched
function ball:update()
    if self.attached then
        local x, y = paddle:getPosition()
        self:setPosition(x + 1.5, y + 1)
        self:setVelocity(0, 0)
    end
end

-- simulate friction between the paddle and the ball
-- TODO add friction coefficient to do this automatically in-engine
function ball:collided(hit)
    if hit == paddle then
        local padvelx = paddle:getVelocity()
        local velx, vely = self:getVelocity()
        self:setVelocity(velx + padvelx * 0.5, vely)
    end
end

-- helper function to create a brick
bricks = {}
function addBrick(x, y, color)
    local brick = Actor
    {
        graphics = SpriteGraphics{sprite="square.tga", color=color},
        collider = AabbCollider{group=8, mask=1},
        position = {x, y},
        scale = {2, 1}
    }
    game:addActor(brick)
    function brick:collided()
        game:removeActor(self)
        for i, v in ipairs(bricks) do
            if v == self then
                table.remove(bricks, i)
                break
            end
        end
    end
    bricks[#bricks+1] = brick
end

-- helper function to generate a color pattern
function genColor(col, row, cols, rows)
    local red = (cols - col) / cols
    local green = col / cols
    local blue = row / rows
    return {red, green, blue}
end

function resetGame()
    -- reset paddle position
    paddle:setPosition(8, 0)
    paddle.vel = 0
    paddle:update()

    -- reset ball position
    ball.attached = true
    ball:update()

    -- clear any bricks currently in play
    for _, v in ipairs(bricks) do
        game:removeActor(v)
    end
    bricks = {}

    -- populate bricks
    for row = 0, 3 do
        for col = 0, 8 do
            addBrick(2 * col + 1, row + 10, genColor(col, row, 8, 3))
        end
    end
end

-- key/gamepad input callbacks
function movePaddle(down, vel)
    if down then
        paddle.vel = vel
    elseif paddle.vel == vel then
        paddle.vel = 0
    end
end

function launchBall()
    if ball.attached then
        local velx = paddle:getVelocity()
        ball:setVelocity(velx, 6)
        ball.attached = false
    end
end

-- helper functions for key/gamepad events
function keyChanged(method, arg)
    return function(down)
        method(down, arg)
    end
end

function keyDown(method, arg)
    return function(down)
        if down then method(arg) end
    end
end

registerControl("right", keyChanged(movePaddle, 12))
registerControl("left", keyChanged(movePaddle, -12))
registerControl("action", keyDown(launchBall))

resetGame()
