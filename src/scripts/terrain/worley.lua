-- Up ahead is my own attempt at a world generator.
-- Be warned, it may be terrible
-- This is heavily baed on my Godot world generator
GenName = "Worley"
GenApiVersion = 3

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
    return c
end

function PopulateChunk(cx,cz)
    -- Secondly, trees
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            fx = cx*16 + x + 8
            fz = cz*16 + z + 8
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