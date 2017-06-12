canvasSize = {20, 15}

tiles16 = TileSet
{
    filename = "tiles16.tga",
    size = {10, 9}
}

tiles32 = TileSet
{
    filename = "tiles32.tga",
    size = {8, 3}
}

custom1 = TileSet
{
    filename = "custom1.tga",
    size = {12, 9}
}

custom2 = TileSet
{
    filename = "custom2.tga",
    size = {12, 8}
}

custom3 = TileSet
{
    filename = "custom3.tga",
    size = {12, 9}
}

alternate = TileSet
{
    filename = "alternate.tga",
    size = {12, 10}
}

sets = {tiles16, tiles32, custom1, custom2, custom3, alternate}

maps = {}

for i, v in ipairs(sets) do
    local cols, rows = v:getSize()
    local map = TileMap
    {
        tileset = v,
        size = {cols, rows}
    }
    local tile = 1
    for y = 0, rows-1 do
        for x = 0, cols-1 do
            map:setTiles(x, y, 1, 1, tile)
            tile = tile + 1
        end
    end
    maps[i] = map
end

palettePicker = Canvas
{
    camera = Camera2D
    {
        size = {#maps, 1},
        fixed = true,
        visible = false
    }
}
addCanvas(palettePicker)

palettes = {}

for i, v in ipairs(maps) do
    local cols, rows = v:getSize()
    palette = Actor
    {
        graphics = TiledGraphics{tilemap = v},
        transform = {position = {i-1, 0}, scale = {1/cols, 1/rows}},
        members =
        {
            index = i,
            onClick = function(self, down)--, x, y)
                if down then
                    curPalette = self.index
                    refresh()
                    return true
                end
            end
        }
    }
    palettePicker:addActor(palette)
    palettes[i] = palette
end

tilePicker = Canvas
{
    camera = Camera2D
    {
        size = {1, 1},
        fixed = true,
        visible = false
    }
}
addCanvas(tilePicker)

tilePickerActor = Actor
{
    members =
    {
        onClick = function(self, down, x, y)
            if down then
                x = math.floor(x)
                y = math.floor(y)
                curTile = self:getGraphics():getTileMap():getTile(x, y)
                refresh()
                return true
            end
        end,
        setPalette = function(self, index)
            local map = maps[index]
            self:setGraphics(TiledGraphics{tilemap = map})
            local cols, rows = map:getSize()
            self:setScale(1/cols, 1/rows)
        end
    }
}
tilePicker:addActor(tilePickerActor)

curTile = 1
curPalette = 1

tilePickerActor:setPalette(curPalette)

painter = Canvas
{
    camera = Camera2D
    {
        size = canvasSize,
        fixed = true
    }
}
addCanvas(painter)

painterActor = Actor
{
    graphics = TiledGraphics{tilemap = TileMap{size = canvasSize}},
    members =
    {
        onClick = function(self, down, x, y)
            if down then
                x = math.floor(x)
                y = math.floor(y)
                self:getGraphics():getTileMap():setTiles(x, y, 1, 1, curTile)
                return true
            end
        end,
        fillTiles = function(self)
            map = self:getGraphics():getTileMap()
            local cols, rows = map:getSize()
            map:setTiles(0, 0, cols, rows, curTile)
        end
    }
}
painter:addActor(painterActor)

canvases = {painter, palettePicker, tilePicker}

function refresh()
    tilePickerActor:setPalette(curPalette)
    painterActor:getGraphics():getTileMap():setTileSet(sets[curPalette])
    for i, v in ipairs(canvases) do
        v:setVisible(false)
    end
    painter:setVisible(true)
end

refresh()

function showPicker(picker)
    return function(down)
        if down then
            for i, v in ipairs(canvases) do
                v:setVisible(false)
            end
            picker:setVisible(true)
        end
    end
end

function actorCaller(actor, call)
    return function(down)
        if down then
            call(actor)
        end
    end
end

registerControl("a", showPicker(palettePicker))
registerControl("s", showPicker(tilePicker))
registerControl("d", actorCaller(painterActor, painterActor.fillTiles))
