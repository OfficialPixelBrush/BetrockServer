GenName = "Perlin"
GenApiVersion = 4

-- Terrain generation using seeded Perlin noise
function GenerateChunk(cx,cz)
    biomes = getBiomeMap(cx,cz)
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
            biome = biomes[x+1][z+1]
            for y = CHUNK_HEIGHT, 0, -1 do
                if (blocksSinceSkyVisible < 6) then
                    if (c[index(x, y, z)][1] == 1) then
                        if (biome == BIOME.DESERT or biome == BIOME.SAVANNA) then
                            c[index(x, y, z)] = {12,0}
                        elseif (biome == BIOME.TAIGA or biome == BIOME.TUNDRA) then
                            c[index(x, y, z)] = {80,0}
                        else
                            c[index(x, y, z)] = {3,0}
                        end
                        blocksSinceSkyVisible = blocksSinceSkyVisible+1
                    end
                end
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
                if (block == nil) then
                    break
                end
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