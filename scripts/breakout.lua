-- create a 20x15 game window
local game = Canvas{size = {20, 15}, fixed = true}
addCanvas(game)

local wallHit = function(self, hit)
    if hit == ball then
        playSample("drum-kick.wav")
    end
end

-- put invisible walls around the edges of the screen
local leftWall = Actor
{
    collider = AabbCollider{group = 4, mask = 3},
    transform = {position = {-1, 0}, scale = {1, 16}},
    members = {onCollide = wallHit}
}
game:addActor(leftWall)

local rightWall = Actor
{
    collider = AabbCollider{group = 4, mask = 3},
    transform = {position = {20, 0}, scale = {1, 16}},
    members = {onCollide = wallHit}
}
game:addActor(rightWall)

local topWall = Actor
{
    collider = AabbCollider{group = 4, mask = 1},
    transform = {position = {0, -1}, scale = {20, 1}},
    members = {onCollide = wallHit}
}
game:addActor(topWall)

local bottomWall = Actor
{
    collider = AabbCollider{group = 4, mask = 1},
    transform = {position = {0, 16}, scale = {20, 1}},
    members =
    {
        -- reset the ball if it hits the bottom wall
        onCollide = function (self, hit)
            if hit == ball then
                ball.attached = true
                ball:onUpdate()
            end
        end
    }
}
game:addActor(bottomWall)

-- create the paddle
-- TODO can we have COR=0 between paddle-walls while still having COR=1 between paddle-ball and ball-walls?
-- TODO make paddle a top edge only collider
paddle = Actor
{
    graphics = SpriteGraphics{sprite = "square.tga"},
    collider = AabbCollider{group = 2, mask = 5},
    physics = {mass = math.huge},
    transform = {scale = {4, 1}},
    members =
    {
        vel = 0,
        onUpdate = function (self)
            velx = self:getVelocity()
            self:addAcceleration((self.vel - velx) * 5, 0)
        end
    }
}
game:addActor(paddle)

-- create the ball
ball = Actor
{
    graphics = SpriteGraphics{sprite = "round.tga"},
    collider = AabbCollider{group = 1},
    physics = {},
    members =
    {
        -- keep the ball stuck to the paddle until it is launched
        onUpdate = function(self)
            if self.attached then
                local x, y = paddle:getPosition()
                self:setPosition(x + 1.5, y - 1)
                self:setVelocity(0, 0)
            end
        end,

        -- simulate friction between the paddle and the ball
        -- TODO add friction coefficient to do this automatically in-engine
        onCollide = function(self, hit)
            if hit == paddle then
                local padvelx = paddle:getVelocity()
                local velx, vely = self:getVelocity()
                self:setVelocity(velx + padvelx * 0.5, vely)
                playSample("drum-kick.wav")
            end
        end
    }
}
game:addActor(ball)

-- helper function to create a brick
bricks = {}

local brick_collided = function(self)
    game:removeActor(self)
    for i, v in ipairs(bricks) do
        if v == self then
            table.remove(bricks, i)
            playSample("drum-kick.wav")
            break
        end
    end
end

function addBrick(x, y, color)
    local brick = Actor
    {
        graphics = SpriteGraphics{sprite="square.tga", color=color},
        collider = AabbCollider{group=8, mask=1},
        transform = {position = {x, y}, scale = {2, 1}},
        members = {onCollide = brick_collided}
    }
    game:addActor(brick)

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
    paddle:setPosition(8, 14)
    paddle.vel = 0
    paddle:onUpdate()

    -- reset ball position
    ball.attached = true
    ball:onUpdate()

    -- clear any bricks currently in play
    for _, v in ipairs(bricks) do
        game:removeActor(v)
    end
    bricks = {}

    -- populate bricks
    for row = 0, 3 do
        for col = 0, 8 do
            addBrick(2 * col + 1, row + 1, genColor(col, row, 8, 3))
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
        ball:setVelocity(velx, -6)
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
registerControl("up", keyDown(saveState))

resetGame()
