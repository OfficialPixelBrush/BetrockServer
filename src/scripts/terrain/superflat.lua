GenName = "Superflat"
GenApiVersion = 3

function GenerateChunk(cx,cz)
    local c = {}
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            for y = 0, 4 do
                type = 0
                if y == 0 then
                    type = 7
                elseif y > 0 and y < 3 then
                    type = 3
                elseif y == 3 then
                    type = 2
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