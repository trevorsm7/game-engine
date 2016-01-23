-- make actor rotate CCW centered around its previous position
local function animate(self, angle, radius)
    self.cx, self.cy = self:getPosition()

    self.update = function(self, delta)
        angle = angle + delta
        local x = math.cos(angle) * radius
        local y = math.sin(angle) * radius
        self:setPosition(self.cx + x, self.cy + y)
    end

    self.move = function(self, x, y)
        self.cx = self.cx + x
        self.cy = self.cy + y
    end
end

-- create a canvas on the left side of window to hold animated actors
--canvas1 = Canvas.create{0, 0, 320, 240}
local canvas1 = Canvas.create{0, 0, 0, 0}

-- spawn 4 animated actors at 0, 90, 180, and 270 degrees
local animated = {}
for i=1,6 do
    local actor = Actor.create(i)
    actor:setScale(16, 16)
    --actor:setPosition(-8, -8)
    animate(actor, i * math.pi/3, 64)
    canvas1:addActor(actor)
    animated[i] = actor
end

-- create an actor that can be moved with arrow keys
local actor3 = Actor.create(3)
actor3:setScale(16, 16)
actor3.velx = 0
actor3.vely = 0
actor3.applyVel = function(self, velx, vely, apply)
    self.velx = self.velx + (apply and velx or -velx)
    self.vely = self.vely + (apply and vely or -vely)
end
actor3.move = function(self, x, y)
    local cx, cy = self:getPosition()
    self:setPosition(cx + x, cy + y)
end
actor3.update = function(self, delta)
    delta = delta * 128
    self:move(self.velx * delta, self.vely * delta)
end
actor3.mouse = function(self, down)
    self:move(-32, -32)
end

-- create second canvas (tiled on right side of window) to hold movable actor
--local canvas2 = Canvas.create{320, 0, 0, 0}
--canvas2:addActor(actor3)
canvas1:addActor(actor3)

local center = {0, 0}
local function resizeCallback(width, height)
    local newcenter = {width / 2, height / 2}
    for i,v in ipairs(animated) do
        v:move(newcenter[1] - center[1], newcenter[2] - center[2])
    end
    center = newcenter
    actor3:setPosition(newcenter[1], newcenter[2])
end
registerResize(resizeCallback)

-- factory function to create a canvas pause toggle callback
local function toggleFactory(obj, fn, init)
    local toggle = init
    fn(obj, toggle)
    return function(down)
        if (down) then
            toggle = not toggle
            fn(obj, toggle)
        end
    end
end

-- register callbacks for keyboard events
registerKey("key_space", toggleFactory(canvas1, canvas1.setPaused, false))
registerKey("key_enter", toggleFactory(actor3, actor3.setVisible, true))
registerKey("key_a", function(down) actor3:applyVel(-1, 0, down) end)
registerKey("key_d", function(down) actor3:applyVel(1, 0, down) end)
registerKey("key_w", function(down) actor3:applyVel(0, 1, down) end)
registerKey("key_s", function(down) actor3:applyVel(0, -1, down) end)
--registerKey("key_tab", function(down) actor3 = animated[1] end)
registerKey("key_esc", quit)
