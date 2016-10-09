table1 = {"a", Actor, math.random}

hello = "Hello"
table2 = table1

mult = function(x, y)
    return x * y
end

mult2 = mult

local hidden = Canvas{size={8, 6}, fixed=true}
addCanvas(hidden)

player = Actor
{
    graphics = SpriteGraphics{sprite="hero.tga"}
}
hidden:addActor(player)

player.table = table1
player.hello = hello
player.mult = mult
player.func = Actor.getGraphics

local meta = {__metatable = "can't touch this"}
meta.__index = meta
setmetatable(table1, meta)

--print("✄")
--print("\226\156\132")

_G[1] = "non-string global key"
_G[mult] = function() print("function with global function key") end
_G[_G[mult]] = function() print("function with temp global function key") end

local temp = {"???"}
_G[Actor.getCollider] = temp
--table1.temp = temp

local tableKey = {}
table1[tableKey] = {tableKey}

a = 4503599627370496e-20
--[[
print(a)
local packed = string.pack("<d", a)
print(packed)
print(string.unpack("<d", packed))--]]

local gen = function(i)
    return function()
        return i
    end
end

local closure1 = gen(2)
local closure2 = gen(3)

player.update = function()
    local a = closure1()
    local b = closure2()
    print("result", mult(a, b))
    saveState()
    quit()
end

table1[player.update] = "function key"
table1[Actor.getGraphics] = "global key"
