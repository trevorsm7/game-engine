test = {"a", Actor, math.random}

test2 = "Hello"
test3 = test

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

player.test = test
player.test2 = test2
player.mult = mult
player.func = Actor.getGraphics

serialize()
quit()
