local red = {1, 0, 0}
local green = {0, 1, 0}
local blue = {0, 0, 1}
local gold = {1, 1, 0}

-- use a different seed in each game
math.randomseed(os.time())

-- simple collision detection method over all objects added to list
local bodies = {}
local function getCollision(x, y)
    for i, body in ipairs(bodies) do
        local bx, by = body:getPosition()
        if body:isVisible() and bx == x and by == y then
            return body
        end
    end
end

-- get a random position guranteed to be empty
local function getEmptySpace()
    local x, y
    repeat
        x = math.random(0, 19) * 32
        y = math.random(0, 14) * 32
    until not getCollision(x, y)
    return x, y
end

god = Actor.create()
god:setVisible(false)
god:setScale(640, 480)
god.score = 0
function god:mouse(down)
    if down and not self.dead then
        io.write("God is dead! Score: ", self.score, "\n")
        io.flush() -- even this doesn't seem to help all the time???
        self.dead = true
    end
    return true
end

-- eating apples makes more sense if you're a worm, but what are you gonna do?
local function newApple(canvas)
    local defaultColor = green
    local defaultScore = 2

    local apple = Actor.create()
    table.insert(bodies, apple)
    canvas:addActor(apple)
    apple:setColor(defaultColor)
    apple:setPosition(getEmptySpace())
    apple:setScale(31, 31)
    apple.food = true
    apple.score = defaultScore

    function apple:eaten()
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
        self:setPosition(getEmptySpace())
    end

    -- godmode: eat apples by clicking on them
    function apple:mouse(down)
        if down and not god.dead then
            god.score = god.score + self.score
            self:eaten()
        end
        return true
    end

    return apple
end

cannibalism = false

-- create an invisible controller that drives a snake
local function newSnake(canvas, name, pos, dir, color)
    local snake = Actor.create()
    snake:setVisible(false)
    canvas:addActor(snake)
    snake.time = 0
    snake.name = name
    snake.length = 5
    --snake.oldDir = {nil, nil} -- this allows any direction on 1st move
    snake.oldDir = {0, 0} -- this forces fwd movement on 1st move
    snake.dir = dir

    local function newBody()
        local body = Actor.create(color)
        table.insert(bodies, body)
        canvas:addActor(body)
        body:setScale(31, 31)
        body.score = 1

        function body:eaten()
            -- disconnect all body members trailing this one
            if self.next then
                local temp1 = snake.tail
                local count = 1
                while temp1 ~= self do
                    local temp2 = temp1.next
                    temp1.next = nil
                    temp1:setVisible(false)
                    temp1 = temp2
                    count = count + 1
                end
                snake.tail = self.next
                snake.curLength = snake.curLength - count
                snake.length = snake.length - count
            end
            self:setVisible(false)
        end

        return body
    end

    -- give the snake one initial segment
    snake.head = newBody()
    snake.head:setPosition(pos[1], pos[2])
    snake.head.food = false
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
        if cannibalism then
            self.head.food = true
        end
        self.dead = true
    end

    -- put most of the snake logic in the update function
    -- NOTE: takes the canvas reference as a closure
    function snake:update(delta)
        -- first check if we're dead; if so, do nothing
        if self.dead then return end

        -- gate snake updates to once per period
        local period = 9 / (self.length + 5)
        self.time = self.time + delta
        if self.time < period then return end
        self.time = self.time - period

        -- get the new head position
        local dx, dy = self.head:getPosition()
        dx = dx + self.dir[1] * 32
        dy = dy + self.dir[2] * 32

        -- see if we run off the screen
        if dx < 0 or dy < 0 or dx >= 640 or dy >= 480 then
            self:die()
            return
        end

        -- see if we collide with a body or with food
        hit = getCollision(dx, dy)
        if hit then
            if hit.food then
                snake.length = snake.length + hit.score
                hit:eaten()
            --elseif hit ~= self.tail then
            else
                self:die()
                return
            end
        end

        -- either create a new head or recycle a tail from the queue
        local head
        if (self.curLength < self.length) then
            --head = Actor.create(color)
            --table.insert(bodies, head)
            --canvas:addActor(head)
            head = newBody()
            head.food = false
            self.curLength = self.curLength + 1
        else
            if self.head == self.tail then
                self:die()
                return
            end
            head = self.tail
            head.food = false
            self.tail = head.next
            head.next = nil
        end

        -- make the new head the current head of the snake
        head:setPosition(dx, dy)
        self.oldDir = self.dir
        if cannibalism then
            self.head.food = true
        end
        self.head.next = head
        self.head = head
    end

    return snake
end

game = Canvas.create{0, 0, 0, 0}

game:addActor(god)

player1 = newSnake(game, "Red", {32, 224}, {1, 0}, red)
registerKey("key_a", player1:moveFactory{-1, 0})
registerKey("key_d", player1:moveFactory{1, 0})
registerKey("key_s", player1:moveFactory{0, -1})
registerKey("key_w", player1:moveFactory{0, 1})

player2 = newSnake(game, "Blue", {576, 224}, {-1, 0}, blue)
registerKey("key_left", player2:moveFactory{-1, 0})
registerKey("key_right", player2:moveFactory{1, 0})
registerKey("key_down", player2:moveFactory{0, -1})
registerKey("key_up", player2:moveFactory{0, 1})

newApple(game)
newApple(game)
