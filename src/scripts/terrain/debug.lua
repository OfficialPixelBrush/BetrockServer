GenName = "Debug"
GenApiVersion = 3

function GenerateBlock(x,y,z)
    local type = 0
    local meta = 0
    if (y == 64) then
        if (x <= 96*2 and x >= 0) then
            if (z < 16*2 and z >= 0) then
                if (x % 2 == 0) then
                    if (z % 2 == 0) then
                        type = x/2
                        meta = z/2
                    end
                end
            else 
                type = 1;
            end
        else 
            type = 1;
        end
    end
    if (y < 64) then
        type = 1;
        meta = 0;
    end
    return {type,meta}
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

function PopulateChunk(cx,cz)
    -- Nothing
end