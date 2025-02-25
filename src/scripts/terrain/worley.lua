-- Up ahead is my own attempt at a world generator.
-- Be warned, it may be terrible
-- This is heavily baed on my Godot world generator
GenName = "Worley"
GenApiVersion = 2

function PlaceTree(c,x,y,z)
    for h = 0, 1 do
        for w = -2, 2 do
            for l = -2, 2 do
                c[index(x+w,y+3+h,z+l)] = {18,0}
            end
        end
    end
    for h = 0, 1 do
        c[index(x+1,y+5+h,z)] = {18,0}
        c[index(x-1,y+5+h,z)] = {18,0}
        c[index(x,y+5+h,z+1)] = {18,0}
        c[index(x,y+5+h,z-1)] = {18,0}
    end

    for h = 0, 5 do
        c[index(x,y+h,z)] = {17,0}
    end
    c[index(x,y+6,z)] = {18,0}
end

function GenerateChunk(cx,cz)
    local c = {}
    -- First we generate rough terrain
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
                    else 
                        roughNoise = getNoiseWorley(cx*16+x,128-y-10,cz*16+z, 80, 1, 0.15, 1);
                        if (roughNoise > 0.5) then
                            solid = false
                        end

                        if (solid) then
                            type = 3 -- getNaturalGrass(x,y,z,blocksSinceSkyVisible)
                        elseif (y < 64) then
                            type = 9
                        end
                    end
                end
                c[index(x, y, z)] = {type, 0}
            end
        end
    end
    -- Secondly, trees
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            -- Iterate from sky to bottom
            for y = CHUNK_HEIGHT, 0, -1 do
                if (c[index(x,y,z)][1] == 9) then
                    break
                end
                -- If we find non-air block
                if (c[index(x,y,z)][1] == 3) then
                    if (spatialPrng(cx*16+x,0,cz*16+z)%64==0) then
                        PlaceTree(c,x,y,z)
                    elseif (spatialPrng(cx*16+x,0,cz*16+z)%5==3) then
                        c[index(x,y+1,z)] = {31,1}
                        c[index(x,y,z)] = {2,0}
                    else
                        c[index(x,y,z)] = {2,0}
                    end
                    break
                end
            end
        end
    end
    return c
end