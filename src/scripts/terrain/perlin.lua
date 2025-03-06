GenName = "Perlin"
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

    for h = 1, 5 do
        c[index(x,y+h,z)] = {17,0}
    end
    c[index(x,y+6,z)] = {18,0}
end

-- Terrain generation using seeded Perlin noise
function GenerateChunk(cx,cz)
    local c = {}
    -- Place blocks
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            local height = math.floor((1.0+getNoisePerlin2d((cx*16+x)*0.01, 0, (cz*16+z)*0.01, 4))*42.0)
            for y = 0, height do
                c[index(x, y, z)] = {1, 0}
            end
            for y = height, CHUNK_HEIGHT do
                if (y < 64) then
                    c[index(x, y, z)] = {9, 0}
                else
                    c[index(x, y, z)] = {0, 0}
                end
            end
        end
    end
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            local blocksSinceSkyVisible = 0;
            for y = CHUNK_HEIGHT, 0, -1 do
                if (blocksSinceSkyVisible < 6) then
                    if (c[index(x, y, z)][1] == 1) then
                        c[index(x, y, z)] = {getNaturalGrass(cx*16+x,y,cz*16+z,blocksSinceSkyVisible),0};
                        blocksSinceSkyVisible = blocksSinceSkyVisible+1
                    end
                end
            end
        end
    end
    -- Lastly, trees
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            -- Iterate from sky to bottom
            for y = CHUNK_HEIGHT, 0, -1 do
                if (c[index(x,y,z)][1] == 9) then
                    break
                end
                -- If we find non-air block
                if (c[index(x,y,z)][1] == 2) then
                    if (spatialPrng(cx*16+x,0,cz*16+z)%64==0) then
                        PlaceTree(c,x,y,z)
                    elseif (spatialPrng(cx*16+x,0,cz*16+z)%5==3) then
                        c[index(x,y+1,z)] = {31,1}
                    end
                    break
                end
            end
        end
    end
    return c
end