game = Canvas {
    size = {40, 30},
    fixed = true
}
addCanvas(game)

mapsize = {40, 30}
rawdata = {}
prettydata = {}

minsize = {4, 3}
maxsize = {12, 10}

function generate()
    -- clear map to all walls
    for i = 1, mapsize[1] * mapsize[2] do
        rawdata[i] = 1
    end

    local root = {x=0, y=0, w=mapsize[1], h=mapsize[2]}
    root.area = (root.w-2) * (root.h-2) -- not really used for root anyway

    --for i = 1, 2 do
    while root.area > 0 do
        local node = root
        while node.room do
            local area = 0
            for j = 1, 4 do
                area = area + node.side[j].area
            end
            local pick = math.random(1, area)
            for j = 1, 4 do
                pick = pick - node.side[j].area
                if pick <= 0 then
                    node = node.side[j]
                    break
                end
            end
        end

        -- Keep 1 unit border around room
        local w = math.random(math.min(minsize[1], node.w-2), math.min(maxsize[1], node.w-2))
        local h = math.random(math.min(minsize[2], node.h-2), math.min(maxsize[2], node.h-2))
        local x = node.x + math.random(1, node.w - w - 1)
        local y = node.y + math.random(1, node.h - h - 1)
        -- TODO handle area < 0

        node.room = {x=x, y=y, w=w, y=y}
        node.side = {}
        --[[node.side[1] = {x=node.x, y=node.y, w=x-node.x, h=y+h-node.y}
        node.side[2] = {x=x, y=node.y, w=node.x+node.w-x, h=y-node.y}
        node.side[3] = {x=x+w, y=y, w=node.x+node.w-(x+w), h=node.y+node.h-y}
        node.side[4] = {x=node.x, y=y+h, w=x+w-node.x, h=node.y+node.h-(y+h)}--]]
        -- add 1 unit padding around room
        node.side[1] = {x=node.x, y=node.y, w=x-node.x-1, h=y+h-node.y+1}
        node.side[2] = {x=x-1, y=node.y, w=node.x+node.w-x+1, h=y-node.y-1}
        node.side[3] = {x=x+w+1, y=y-1, w=node.x+node.w-(x+w)-1, h=node.y+node.h-y+1}
        node.side[4] = {x=node.x, y=y+h+1, w=x+w-node.x+1, h=node.y+node.h-(y+h)-1}

        local area = 0
        for j = 1, 4 do
            node.side[j].parent = node
            -- subtract 1 unit border
            if node.side[j].w >= minsize[1]+2 and node.side[j].h >= minsize[2]+2 then
                node.side[j].area = (node.side[j].w-2) * (node.side[j].h-2)
            else
                node.side[j].area = 0
            end
            area = area + node.side[j].area
            --[[if node.side[j].area > 0 then
                game:debugLine(
                    node.side[j].x, node.side[j].y,
                    node.side[j].x+node.side[j].w, node.side[j].y,
                    node.side[j].x+node.side[j].w, node.side[j].y+node.side[j].h,
                    node.side[j].x, node.side[j].y+node.side[j].h,
                    node.side[j].x, node.side[j].y)
            end--]]
        end

        -- easier to subtract on the way up
        area = node.area - area
        while node do
            node.area = node.area - area
            node = node.parent
        end

        -- Fill room (can do later?)
        for yi = y, y+h-1 do
            yo = yi * mapsize[1]
            for xi = x, x+w-1 do
                rawdata[xi+yo+1] = 0
            end
        end
    end

    -- 8 5 9
    -- 4 1 2
    -- 7 3 6
    local dir = {
        {0, 0},
        {1, 0},
        {0, 1},
        {-1, 0},
        {0, -1},
        {1, 1},
        {-1, 1},
        {-1, -1},
        {1, -1}
    }
    local lut = {
        [0] = {
            [0] = {
                [0] = {
                    [0] = {
                        [0] = {
                            [0] = {
                                [0] = {
                                    [0] = {
                                        [0] = {tile = 14},
                                        [1] = {tile = 19}
                                    },
                                    [1] = {
                                        [0] = {tile = 18},
                                        [1] = {tile = 45}
                                    }
                                },
                                [1] = {
                                    [0] = {
                                        [0] = {tile = 30},
                                        [1] = {tile = 34}
                                    },
                                    [1] = {
                                        [0] = {tile = 48},
                                        [1] = {tile = 36}
                                    }
                                }
                            },
                            [1] = {
                                [0] = {
                                    [0] = {
                                        [0] = {tile = 31},
                                        [1] = {tile = 9}
                                    },
                                    [1] = {
                                        [0] = {tile = 35},
                                        [1] = {tile = 46}
                                    }
                                },
                                [1] = {
                                    [0] = {
                                        [0] = {tile = 12},
                                        [1] = {tile = 21}
                                    },
                                    [1] = {
                                        [0] = {tile = 11},
                                        [1] = {tile = 22}
                                    }
                                }
                            }
                        },
                        [1] = {
                            [0] = {
                                [0] = {tile = 2},
                                [1] = {tile = 6}
                            },
                            [1] = {
                                [0] = {tile = 7},
                                [1] = {tile = 10}
                            }
                        }
                    },
                    [1] = {
                        [0] = {
                            [0] = {
                                dir = {1, -1},
                                [0] = {tile = 13},
                                [1] = {tile = 17}
                            },
                            [1] = {
                                dir = {1, -1},
                                [0] = {tile = 29},
                                [1] = {tile = 33}
                            }
                        },
                        [1] = {
                            [0] = {tile = 1},
                            [1] = {tile = 5}
                        }
                    }
                },
                [1] = {
                    [0] = {
                        [0] = {
                            dir = {-1, -1},
                            [0] = {
                                dir = {1, -1},
                                [0] = {tile = 26},
                                [1] = {tile = 43}
                            },
                            [1] = {
                                dir = {1, -1},
                                [0] = {tile = 42},
                                [1] = {tile = 47}
                            }
                        },
                        [1] = {tile = 38}
                    },
                    [1] = {
                        [0] = {
                            dir = {1, -1},
                            [0] = {tile = 25},
                            [1] = {tile = 41}
                        },
                        [1] = {tile = 37}
                    }
                }
            },
            [1] = {
                [0] = {
                    [0] = {
                        [0] = {
                            dir = {-1, 1},
                            [0] = {
                                dir = {-1, -1},
                                [0] = {tile = 15},
                                [1] = {tile = 20}
                            },
                            [1] = {
                                dir = {-1, -1},
                                [0] = {tile = 32},
                                [1] = {tile = 24}
                            }
                        },
                        [1] = {
                            dir = {-1, 1},
                            [0] = {tile = 3},
                            [1] = {tile = 8}
                        }
                    },
                    [1] = {
                        [0] = {tile = 16},
                        [1] = {tile = 4}
                    }
                },
                [1] = {
                    [0] = {
                        [0] = {
                            dir = {-1, -1},
                            [0] = {tile = 27},
                            [1] = {tile = 44}
                        },
                        [1] = {tile = 39}
                    },
                    [1] = {
                        [0] = {tile = 28},
                        [1] = {tile = 40}
                    }
                }
            }
        },
        [1] = {
            [0] = {
                [0] = {
                    [0] = {
                        [0] = {tile = 88},
                        [1] = {tile = 76}
                    },
                    [1] = {
                        [0] = {tile = 87},
                        [1] = {
                            dir = {-1, -1},
                            [0] = {tile = 92},
                            [1] = {tile = 75}
                        }
                    }
                },
                [1] = {
                    [0] = {
                        [0] = {tile = 52},
                        [1] = {tile = 64}
                    },
                    [1] = {
                        [0] = {
                            dir = {-1, 1},
                            [0] = {tile = 56},
                            [1] = {tile = 51}
                        },
                        [1] = {
                            dir = {-1, 1},
                            [0] = {
                                dir = {-1, -1},
                                [0] = {tile = 72},
                                [1] = {tile = 80}
                            },
                            [1] = {
                                dir = {-1, -1},
                                [0] = {tile = 68},
                                [1] = {tile = 63}
                            }
                        }
                    }
                }
            },
            [1] = {
                [0] = {
                    [0] = {
                        [0] = {tile = 85},
                        [1] = {
                            dir = {1, -1},
                            [0] = {tile = 89},
                            [1] = {tile = 73}
                        }
                    },
                    [1] = {
                        [0] = {tile = 86},
                        [1] = {
                            dir = {-1, -1},
                            [0] = {
                                dir = {1, -1},
                                [0] = {tile = 95},
                                [1] = {tile = 90}
                            },
                            [1] = {
                                dir = {1, -1},
                                [0] = {tile = 91},
                                [1] = {tile = 74}
                            }
                        }
                    }
                },
                [1] = {
                    [0] = {
                        [0] = {
                            [0] = {tile = 53},
                            [1] = {tile = 49}
                        },
                        [1] = {
                            [0] = {
                                dir = {1, -1},
                                [0] = {tile = 81},
                                [1] = {tile = 77}
                            },
                            [1] = {
                                dir = {1, -1},
                                [0] = {tile = 65},
                                [1] = {tile = 61}
                            }
                        }
                    },
                    [1] = {
                        [0] = {
                            [0] = {
                                [0] = {tile = 58},
                                [1] = {tile = 55}
                            },
                            [1] = {
                                [0] = {tile = 54},
                                [1] = {tile = 50}
                            }
                        },
                        [1] = {
                            [0] = {
                                [0] = {
                                    [0] = {
                                        [0] = {tile = 70},
                                        [1] = {tile = 59}
                                    },
                                    [1] = {
                                        [0] = {tile = 69},
                                        [1] = {tile = 60}
                                    }
                                },
                                [1] = {
                                    [0] = {
                                        [0] = {tile = 94},
                                        [1] = {tile = 83}
                                    },
                                    [1] = {
                                        [0] = {tile = 57},
                                        [1] = {tile = 79}
                                    }
                                }
                            },
                            [1] = {
                                [0] = {
                                    [0] = {
                                        [0] = {tile = 84},
                                        [1] = {tile = 96}
                                    },
                                    [1] = {
                                        [0] = {tile = 82},
                                        [1] = {tile = 78}
                                    }
                                },
                                [1] = {
                                    [0] = {
                                        [0] = {tile = 93},
                                        [1] = {tile = 66}
                                    },
                                    [1] = {
                                        [0] = {tile = 67},
                                        [1] = {tile = 62}
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    local w = mapsize[1]
    local h = mapsize[2]
    local i = 1
    for y = 1, h do
        for x = 1, w do
            local node = lut
            for _, v in ipairs(dir) do
                if node.dir then
                    v = node.dir
                end
                local nx = x + v[1]
                local ny = y + v[2]
                local ni = i
                if nx >= 1 and nx <= w then
                    ni = ni + v[1]
                end
                if ny >= 1 and ny <= h then
                    ni = ni + v[2] * w
                end
                local new = node[rawdata[ni]]
                if new then
                    node = new
                else
                    break
                end
            end
            prettydata[i] = node.tile
            i = i + 1
        end
    end
end

generate()

tileset = TileSet {
    filename = "custom3.tga",
    size = {12, 9},
    data =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        0, 0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 0
    }
}

tilemap = TileMap {
    tileset = tileset,
    size = mapsize,
    data = prettydata
}

dungeon = Actor {
    graphics = TiledGraphics{tilemap = tilemap},
    collider = TiledCollider{tilemap = tilemap}
}
game:addActor(dungeon)

pawnA = Actor
{
    graphics = SpriteGraphics{sprite = "hero.tga"},
    transform = {position = {2, 13}}
}
game:addActor(pawnA)

local shadowMode = 1
local visibility = TileMask{size = {tilemap:getSize()}}
local seen = TileMask{size = {tilemap:getSize()}}
local overlay = TileMask{size = {tilemap:getSize()}}
--tilemap:setTileMask(seen)

function updateVisibility(x, y)
    tilemap:castShadows(visibility, x, y, 8, shadowMode)
    overlay:fillCircle(x, y, 8)
    visibility:blendMin(overlay)
    seen:clampMask(0, 127)
    seen:blendMax(visibility)
end

updateVisibility(pawnA:getPosition())

function moveFunc(actor, dx, dy)
    return function(down)
        if down then
            local x, y = actor:getPosition()
            x, y = x + dx, y + dy
            actor:setPosition(x, y)
            updateVisibility(x, y)
        end
    end
end

registerControl("left", moveFunc(pawnA, -1, 0))
registerControl("right", moveFunc(pawnA, 1, 0))
registerControl("down", moveFunc(pawnA, 0, 1))
registerControl("up", moveFunc(pawnA, 0, -1))

registerControl("action",
    function (down)
        if down then
            generate()
            tilemap = TileMap {
                tileset = tileset,
                size = mapsize,
                data = prettydata
            }
            dungeon:setGraphics(TiledGraphics{tilemap = tilemap})
            dungeon:setCollider(TiledCollider{tilemap = tilemap})
        end
    end)

function setMode(arg)
    return function(down)
        if down then
            shadowMode = arg
            updateVisibility(pawnA:getPosition())
        end
    end
end

registerControl("a", setMode(1))
registerControl("s", setMode(2))
registerControl("d", setMode(3))
