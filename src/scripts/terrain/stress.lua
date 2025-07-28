-- Based on glitched terrain gen I got while remaking the worley generator in lua
GenName = "Stress"
GenApiVersion = 3

function GenerateChunk(cx, cz)
    local c = {}
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            for y = 0, CHUNK_HEIGHT do
                local type = math.random(0, 96)
                c[index(x, y, z)] = {type, 0}
            end
        end
    end
    return c
end

function PopulateChunk(cx,cz)
    -- Nothing
end