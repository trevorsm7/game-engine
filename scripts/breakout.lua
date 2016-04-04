-- create a 20x15 game window
game = Canvas.create({20, 15}, true)

-- put invisible walls around the edges of the screen
leftWall = Actor.create{collider=true}
--leftWall = Actor.create{collider=true, physics=true, immobile=true, cof=0}
game:addActor(leftWall)
leftWall:setPosition(-1, -1)
leftWall:setScale(1, 16)

rightWall = Actor.create{collider=true}
--rightWall = Actor.create{collider=true, physics=true, immobile=true, cof=0}
game:addActor(rightWall)
rightWall:setPosition(20, -1)
rightWall:setScale(1, 16)

topWall = Actor.create{collider=true}
--topWall = Actor.create{collider=true, physics=true, immobile=true, cof=0}
game:addActor(topWall)
topWall:setPosition(0, 15)
topWall:setScale(20, 1)

bottomWall = Actor.create{collider=true}
game:addActor(bottomWall)
bottomWall:setPosition(0, -2)
bottomWall:setScale(20, 1)

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
paddle = Actor.create{sprite="square.tga", collider=true, physics=true, mass=math.huge}--, cof=1}
game:addActor(paddle)
paddle:setScale(4, 1)

paddle.vel = 0
function paddle:update()
    velx = self:getVelocity()
    self:addAcceleration((self.vel - velx) * 5, 0)
end

-- create the ball
ball = Actor.create{sprite="round.tga", collider=true, physics=true, mass=1}--, cof=1}
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
    local brick = Actor.create{sprite="square.tga", collider=true}
    game:addActor(brick)
    brick:setPosition(x, y)
    brick:setScale(2, 1)
    brick:setColor(color)
    function brick:collided()
        game:removeActor(self)
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
    for _,v in ipairs(bricks) do
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
        ball:setVelocity(velx, 5)
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
