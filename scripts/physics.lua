colors = {}
colors[1] = {1, 1, 1}
colors[2] = {1, 0, 0}
--colors[3] = {0, 1, 0}
--colors[4] = {0, 0, 1}
--colors[5] = {1, 1, 0}
--colors[6] = {1, 0, 1}
--colors[7] = {0, 1, 1}

function addStaticBox(canvas, x, y)
    local box = Actor.create{sprite="square.tga", collider=true}
    canvas:addActor(box)
    box:setPosition(x, y)
    return box
end

function updateBox(self, delta)
    --self:setVelocity(-2, 0)
    local x, y = self:getPosition()
    if x <= -3 or x >= 3 then
        --self:setPosition(x + 6, y)
        vx, vy = self:getVelocity()
        self:setVelocity(-vx, vy)
    end
end

function collideBox(self)
    local newColor = self.color + 1
    if newColor > #colors then
        newColor = 1
    end
    self.color = newColor
    self:setColor(colors[newColor])
end

function addPhysicsBox(canvas, x, y, m, cor, ay)
    local params = {sprite="square.tga", collider=true, physics=true}
    if m then params.mass = m end
    if cor then params.cor = cor end
    local box = Actor.create(params)
    canvas:addActor(box)
    box:setPosition(x, y)
    if ay then
        function box:update(delta)
            self:addAcceleration(0, ay)
        end
    end
    box.color = 1
    box.collided = collideBox
    --box.update = updateBox
    return box
end

canvases = {}
reset = {}

canvases[1] = Canvas.create({20, 20}, false)
canvases[1]:setCenter(0, 0)

reset[1] = function(canvas)
    addStaticBox(canvas, -5, 0)
    addStaticBox(canvas, 5, 0)

    addPhysicsBox(canvas, -4, 0):setVelocity(3, 0)

    addPhysicsBox(canvas, -1, 0)
    addPhysicsBox(canvas, 0, 0)
    addPhysicsBox(canvas, 1, 0)
    addPhysicsBox(canvas, 2, 0)
end

reset[1](canvases[1])

canvases[2] = Canvas.create({20, 20}, false)
canvases[2]:setCenter(0, 0)
canvases[2]:setVisible(false)
canvases[2]:setPaused(true)

reset[2] = function(canvas)
    addStaticBox(canvas, -5, 0)
    addStaticBox(canvas, 5, 0)

    addPhysicsBox(canvas, -4, 0):setVelocity(3, 0)
    addPhysicsBox(canvas, -3, 0):setVelocity(3, 0)
    --addPhysicsBox(canvas, -2, 0):setVelocity(3, 0)

    addPhysicsBox(canvas, 0, 0)
    addPhysicsBox(canvas, 1, 0)
    addPhysicsBox(canvas, 2, 0)
end

reset[2](canvases[2])

canvases[3] = Canvas.create({20, 20}, false)
canvases[3]:setCenter(0, 0)
canvases[3]:setVisible(false)
canvases[3]:setPaused(true)

reset[3] = function(canvas)
    for col = -5, 5 do
        addStaticBox(canvas, col, -1)
    end
    for col = -1, 1 do
        for row = 0, 5 do
            addPhysicsBox(canvas, col, row, 1, .5, -10)
        end
    end
    addPhysicsBox(canvas, -10, 1, 100, .5):setVelocity(5, 0):setScale(4, 4)
end

reset[3](canvases[3])

--canvas[4] = Canvas.create({30, 30}, false)
canvases[4] = Canvas.create({16, 12}, false)
canvases[4]:setCenter(0, -2)
canvases[4]:setVisible(false)
canvases[4]:setPaused(true)

reset[4] = function(canvas)
    local center = addPhysicsBox(canvas, -1, 1, 1):setVelocity(2, 0)
    function center:update()
        local x, y = self:getPosition()
        canvas:setCenter(x + 0.5, y + 0.5)
    end

    addPhysicsBox(canvas, -1, 2, 1)
    addStaticBox(canvas, -1, 2+3)

    addPhysicsBox(canvas, 2, 1, 1)
    addStaticBox(canvas, 2+5, 1)

    addPhysicsBox(canvas, 1, -2, 1)
    addStaticBox(canvas, 1, -2-7)

    addPhysicsBox(canvas, -2, -1, 1)
    addStaticBox(canvas, -2-5, -1)

    addPhysicsBox(canvas, -2, 1, 1):setVelocity(-2, 0)
    addStaticBox(canvas, -2-5, 1)

    addPhysicsBox(canvas, 1, 2+2, 1):setVelocity(0, -2)
    addStaticBox(canvas, 1, 2+3)

    addPhysicsBox(canvas, 2+4, -1, 1):setVelocity(-2, 0)
    addStaticBox(canvas, 2+5, -1)

    addPhysicsBox(canvas, -1, -2-6, 1):setVelocity(0, 2)
    addStaticBox(canvas, -1, -2-7)
end

reset[4](canvases[4])

canvases[5] = Canvas.create({30, 30}, false)
canvases[5]:setCenter(0, 0)
canvases[5]:setVisible(false)
canvases[5]:setPaused(true)

reset[5] = function(canvas)
    addPhysicsBox(canvas, -2, 2, 1)

    addStaticBox(canvas, -7, 2)
    addPhysicsBox(canvas, -3, 2, 2):setVelocity(3, 0)
    addPhysicsBox(canvas, 3-1, 2, 2):setVelocity(1, 0)
    addStaticBox(canvas, 7, 2)

    addStaticBox(canvas, 2, 7)
    addPhysicsBox(canvas, 2, 3+3, 2):setVelocity(0, -3)
    addPhysicsBox(canvas, 2, -3+2, 2):setVelocity(0, -1)
    addStaticBox(canvas, 2, -7)

    addStaticBox(canvas, 10, -2)
    addPhysicsBox(canvas, 3+6, -2, 2):setVelocity(-3, 0)
    addPhysicsBox(canvas, -3+3, -2, 2):setVelocity(-1, 0)
    addStaticBox(canvas, -10, -2)

    addStaticBox(canvas, -2, -13)
    addPhysicsBox(canvas, -2, -3-9, 2):setVelocity(0, 3)
    addPhysicsBox(canvas, -2, 3-4, 2):setVelocity(0, 1)
    addStaticBox(canvas, -2, 13)
end

reset[5](canvases[5])

local index = 1
function changeIndex(dir)
    local newIndex = index + dir
    if newIndex >= 1 and newIndex <= #canvases then
        canvases[index]:setVisible(false)
        canvases[index]:setPaused(true)
        index = newIndex
        canvases[index]:setVisible(true)
        canvases[index]:setPaused(false)
    end
end

function resetIndex()
    canvases[index]:clear()
    reset[index](canvases[index])
end

function keyDown(method, arg)
    return function(down)
        if down then method(arg) end
    end
end

registerControl("up", keyDown(changeIndex, -1))
registerControl("down", keyDown(changeIndex, 1))
registerControl("action", keyDown(resetIndex))
