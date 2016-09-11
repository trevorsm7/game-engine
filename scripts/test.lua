test = {"a", Actor, math.random}

test2 = "Hello"
test3 = test

local hidden = Canvas{size={8, 6}, fixed=true}
addCanvas(hidden)

player = Actor
{
    graphics = SpriteGraphics{sprite="hero.tga"}
}
hidden:addActor(player)

player.test = player.getGraphics

serialize()
quit()
