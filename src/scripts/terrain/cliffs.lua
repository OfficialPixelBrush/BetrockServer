-- Generates some pretty cool cliffs towards the Left side of spawn!
GenName = "Cliffs"
GenApiVersion = 2

function GenerateBlock(x,y,z)
    local type = 0
    local solid = true
    if (y == 0) then
        type = 7
    elseif y > 0 then
        if (between(y,0,3) and spatialPrng(x,y,z)%2) then
            type = 7
            return type
        end
        if (getNoiseWorley(x,128-y-13,z,x,1.0,0.1,1.0) > 0.2) then
            solid = false
        end
    end
    if (solid) then
        type = 3 -- getNaturalGrass(x,y,z,blocksSinceSkyVisible);
    elseif (y < 64) then
        type = 9
    end
    return {type,0}
end

function GenerateChunk(cx,cz)
    local c = {}
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            for y = 0, CHUNK_HEIGHT do
                c[index(x, y, z)] = GenerateBlock(cx*16+x,y,cz*16+z)
            end
        end
    end
    return c
end