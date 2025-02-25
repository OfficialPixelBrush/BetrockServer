GenName = "Sine"
GenApiVersion = 2

function GenerateChunk(cx,cz)
    local c = {}
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            for y = 0, CHUNK_HEIGHT do
                type = 0
                if y < 50 + math.sin((cx*16+x)*10)*3 then
                    type = 1
                end
                c[index(x, y, z)] = {type, 0}
            end
        end
    end
    return c
end