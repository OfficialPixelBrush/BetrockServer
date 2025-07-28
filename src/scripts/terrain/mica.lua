-- Based on glitched terrain gen I got while remaking the worley generator in lua
GenName = "Mica"
GenApiVersion = 3

function GenerateChunk(cx,cz)
    local c = {}
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            for y = 0, CHUNK_HEIGHT do
                local type = 0
                local solid = true
                if (y == 0) then
                    type = 7
                elseif y > 0 then
                    if (between(y,0,3) and spatialPrng(cx*16+x,y,cz*16+z)%2) then
                        type = 7
                    elseif (getNoiseWorley((cx*16+x)*16,128-y-13,cz*16+z,60,1.0,0.1,1.0) > 0.2) then
                        solid = false
                    end
                end
                if (solid) then
                    value = spatialPrng(cx*16+x,y,cz*16+z)%4;
                    if (value == 0) then
                        type = 4
                    else
                        type = 1
                    end
                elseif (y < 64) then
                    type = 9
                end
                c[index(x, y, z)] = {type, 0}
            end
        end
    end
    return c
end

function PopulateChunk(cx,cz)
    -- Nothing
end