game = Canvas.create({20, 20}, false, {0, 0, 0, 0})

tiles = Actor.create{tiles="tiles.tga"}
game:addActor(tiles)
tiles:setPosition(0, 0)
tiles:setScale(4, 4)
game:setCenter(2, 2)

player = Actor.create{sprite="hero.tga", collider=true}
game:addActor(player)
player:setPosition(-1, -1)

nerd = Actor.create{sprite="nerd.tga", collider=true}
game:addActor(nerd)
nerd:setPosition(4, 4)
