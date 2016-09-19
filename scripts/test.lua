table1 = {"a", Actor, math.random}

hello = "Hello"
foo = 2
bar = 3
table2 = table1

mult = function(x, y)
    return x * y
end

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

--[[local gen = function(i)
    return function()
        return i
        --return "foo"
    end
end

closure1 = gen(1)
closure2 = gen(2)--]]

--[[testload = load("print('it works')")
--testload = load(string.dump(load("print('it works')")))
local temp = string.dump(testload, true)
local temp2 = "\27LuaS\0\25\147\13\n\26\n\4\8\4\8\8xV\0\0\0\0\0\0\0\0\0\0\0(w@\1\0\0\0\0\0\0\0\0\0\0\2\2\4\0\0\0\6\0@\0A@\0\0$@\0\1&\0\128\0\2\0\0\0\4\6print\4\9it works\1\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
print("temp", string.len(temp), "temp2", string.len(temp2))
for i = 1, string.len(temp) do
    local t1 = string.byte(temp, i)
    local t2 = string.byte(temp2, i)
    if t1 ~= t2 then
        print("diff at ", i, t1, t2)
    end
end
print(temp)
print(temp2)
testload = load(temp2)
--testload = load("\27LuaS\0\25?\13\n\26\n\4\8\4\8\8xV\0\0\0\0\0\0\0\0\0\0\0(w@\1\18print('it works')\0\0\0\0\0\0\0\0\0\2\2\4\0\0\0\6\0@\0A@\0\0$@\0\1&\0?\0\2\0\0\0\4\6print\4\9it works\1\0\0\0\1\0\0\0\0\0\4\0\0\0\1\0\0\0\1\0\0\0\1\0\0\0\1\0\0\0\0\0\0\0\1\0\0\0\5_ENV", "testload", "b", _G)
--testload = load("\27LuaS\0\25?\13\n\26\n\4\8\4\8\8xV\0\0\0\0\0\0\0\0\0\0\0(w@\1\0\0\0\0\0\0\0\0\0\0\2\2\4\0\0\0\6\0@\0A@\0\0$@\0\1&\0?\0\2\0\0\0\4\6print\4\9it works\1\0\0\0\1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0")
--print(testload)
testload()--]]

player.update = function()
    print("result", mult(foo, bar))
    serialize()
    quit()
end
