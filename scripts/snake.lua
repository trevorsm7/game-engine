local red = {1, 0, 0}
local green = {0, 1, 0}
local blue = {0, 0, 1}
local gold = {1, 1, 0}

-- get a random position guranteed to be empty
local function getEmptySpace(canvas)
    local x, y
    repeat
        x = math.random(0, 19)
        y = math.random(0, 14)
    until not canvas:getCollision(x, y)
    return x, y
end

-- eating apples makes more sense if you're a worm, but what are you gonna do?
local function newApple(canvas)
    local defaultColor = green
    local defaultScore = 2

    local apple = Actor.create{color=defaultColor, sprite="round.tga", collider=true}
    canvas:addActor(apple)
    apple:setPosition(getEmptySpace(canvas))
    apple.food = true
    apple.score = defaultScore

    function apple:eaten()
        local canvas = self:getCanvas()
        if not canvas then return end

        -- every so often, add an extra apple to the game CHAOS
        if math.random() < 0.1 then
            newApple(canvas, green)
        end

        -- occasionally spawn a golden apple
        if math.random() < 0.05 then
            apple:setColor(gold)
            apple.score = 5
        else
            apple:setColor(defaultColor)
            apple.score = defaultScore
        end

        -- then move the apple to a new space
        self:setPosition(getEmptySpace(canvas))
    end

    -- godmode: eat apples by clicking on them
    function apple:mouse(down)
        if down and god and not god.dead then
            god.score = god.score + self.score
            self:eaten()
        end
        return true
    end

    return apple
end

-- create an invisible controller that drives a snake
local function newSnake(canvas, name, pos, dir, color)
    local snake = Actor.create{}
    snake:setVisible(false)
    canvas:addActor(snake)
    snake.time = 0
    snake.name = name
    snake.length = 5
    --snake.oldDir = {nil, nil} -- this allows any direction on 1st move
    snake.oldDir = {0, 0} -- this forces fwd movement on 1st move
    snake.dir = dir

    local function newBody(canvas, x, y)
        local body = Actor.create{color=color, sprite="round.tga", collider=true}
        canvas:addActor(body)
        body:setPosition(x, y)
        return body
    end

    -- give the snake one initial segment
    snake.head = newBody(canvas, pos[1], pos[2])
    snake.tail = snake.head
    snake.curLength = 1

    -- generate a callback function to move the snake in a direction
    function snake:moveFactory(dir)
        return function(down)
            if (down and self.oldDir[1] ~= -dir[1] and self.oldDir[2] ~= -dir[2]) then
                self.dir = dir
            end
        end
    end

    function snake:die()
        io.write(self.name, " is dead! Score: ", self.length, "\n")
        io.flush() -- even this doesn't seem to help all the time???
        self.dead = true
    end

    -- put most of the snake logic in the update function
    function snake:update(delta)
        local canvas = self:getCanvas()
        if not canvas then return end

        -- first check if we're dead; if so, do nothing
        if self.dead then return end

        -- gate snake updates to once per period
        local period = 9 / (self.length + 5)
        self.time = self.time + delta
        if self.time < period then return end
        self.time = self.time - period

        -- get the new head position
        local dx, dy = self.head:getPosition()
        dx = dx + self.dir[1]
        dy = dy + self.dir[2]

        -- see if we run off the screen
        if dx < 0 or dy < 0 or dx >= 20 or dy >= 15 then
            self:die()
            return
        end

        -- see if we collide with a body or with food
        hit = canvas:getCollision(dx, dy)
        if hit then
            if hit.food then
                snake.length = snake.length + hit.score
                hit:eaten()
            else
                self:die()
                return
            end
        end

        -- either create a new head or recycle a tail from the queue
        local head
        if (self.curLength < self.length) then
            head = newBody(canvas, dx, dy)
            self.curLength = self.curLength + 1
        else
            if self.head == self.tail then
                self:die()
                return
            end
            head = self.tail
            head:setPosition(dx, dy)
            self.tail = head.next
            head.next = nil
        end

        -- make the new head the current head of the snake
        self.oldDir = self.dir
        self.head.next = head
        self.head = head
    end

    return snake
end

local function resetGame(game)
    game:clear()

    god = Actor.create{layer=-1}
    game:addActor(god)
    god:setVisible(false)
    god:setScale(20, 15)
    god.score = 0

    function god:mouse(down)
        if down and not self.dead then
            io.write("God is dead! Score: ", self.score, "\n")
            io.flush() -- even this doesn't seem to help all the time???
            self.dead = true
        end
        return true
    end

    local player1 = newSnake(game, "Red", {1, 7}, {1, 0}, red)
    registerControl("a", player1:moveFactory{-1, 0})
    registerControl("d", player1:moveFactory{1, 0})
    registerControl("s", player1:moveFactory{0, -1})
    registerControl("w", player1:moveFactory{0, 1})

    local player2 = newSnake(game, "Blue", {18, 7}, {-1, 0}, blue)
    registerControl("left", player2:moveFactory{-1, 0})
    registerControl("right", player2:moveFactory{1, 0})
    registerControl("down", player2:moveFactory{0, -1})
    registerControl("up", player2:moveFactory{0, 1})

    newApple(game)
    newApple(game)
end

-- set a fresh random seed when we start up
math.randomseed(os.time())

-- create and populate our game canvas
local game = Canvas.create({20, 15}, true)
resetGame(game)

-- create game menu
local menu = Canvas.create({20, 15}, true)
menu:setCenter(0, 0)
menu:setVisible(false)

local continue = Actor.create{sprite="continue.tga"}
menu:addActor(continue)
continue:setPosition(-5, 2)
continue:setScale(10, 2)
function continue:mouse(down)
    if (down) then
        menu:setVisible(false)
        game:setPaused(false)
    end
    return true
end

local newGame = Actor.create{sprite="newgame.tga"}
menu:addActor(newGame)
newGame:setPosition(-5, -1)
newGame:setScale(10, 2)
function newGame:mouse(down)
    if (down) then
        resetGame(game)
        menu:setVisible(false)
        game:setPaused(false)
    end
    return true
end

local endGame = Actor.create{sprite="quit.tga"}
menu:addActor(endGame)
endGame:setPosition(-5, -4)
endGame:setScale(10, 2)
function endGame:mouse(down)
    if (down) then
        quit()
    end
    return true
end

registerControl("quit", function (down)
    if down then
        menu:setVisible(true)
        game:setPaused(true)
    end
end)
