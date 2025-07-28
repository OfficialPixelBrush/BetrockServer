-- Generates some pretty cool cliffs towards the Left side of spawn!
GenName = "Cliffs"
GenApiVersion = 3

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

function PlaceTree(x,y,z)
    for h = 0, 1 do
        for w = -2, 2 do
            for l = -2, 2 do
                placeBlock(x+w,y+3+h,z+l,{18,0});
            end
        end
    end
    for h = 0, 1 do
        placeBlock(x+1,y+5+h,z,{18,0});
        placeBlock(x-1,y+5+h,z,{18,0});
        placeBlock(x,y+5+h,z+1,{18,0});
        placeBlock(x,y+5+h,z-1,{18,0});
    end

    for h = 1, 5 do
        placeBlock(x,y+h,z,{17,0});
    end
    placeBlock(x,y+6,z,{18,0});
end

function PopulateChunk(cx,cz)
    -- Secondly, trees
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            fx = cx*16 + x
            fz = cz*16 + z
            -- Iterate from sky to bottom
            for y = CHUNK_HEIGHT, 0, -1 do
                block = getBlock(fx,y,fz)
                if (block[1] == 9) then
                    break
                end
                -- If we find non-air block
                if (block[1] == 3) then
                    if (spatialPrng(cx*16+fx,0,cz*16+fz)%64==0) then
                        PlaceTree(fx,y,fz)
                    elseif (spatialPrng(cx*16+fx,0,cz*16+fz)%5==3) then
                        placeBlock(fx,y+1,fz,{31,1});
                        placeBlock(fx,y,fz,{2,0});
                    else
                        placeBlock(fx,y,fz,{2,0});
                    end
                    break
                end
            end
        end
    end
    return true
end