mapsize = {40, 30}
rawdata = {}
prettydata = {}

for i = 1, mapsize[1] * mapsize[2] do
    rawdata[i] = 1
end

for i = 1, 40 do
    w = math.random(3, 6)
    h = math.random(3, 6)
    x = math.random(1, mapsize[1]-w)
    y = math.random(1, mapsize[2]-h)
    for yi = 1, h do
        yo = (y+yi-2) * mapsize[1]
        for xi = 1, w do
            xo = x+xi-1
            rawdata[xo+yo] = 0
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
                            [0] = {tile = 64},
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

tilemap = TileMap {
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
    },
    size = mapsize,
    data = prettydata
}

game = Canvas {
    size = {40, 30},
    fixed = true
}
addCanvas(game)

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
tilemap:setTileMask(seen)

function updateVisibility(x, y)
    tilemap:castShadows(visibility, x, y, 8, shadowMode)
    overlay:fillCircle(x, y, 8)
    visibility:blendMin(overlay)

    seen:clampMask(0, 127)
    seen:blendMax(visibility)
    --overlay:fillCircle(x, y, 5, 255, 120)
    --seen:blendMin(overlay)
    --overlay:fillCircle(x, y, 4, 255, 160)
    --seen:blendMin(overlay)
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
