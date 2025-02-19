-- Up ahead is my own attempt at a world generator.
-- Be warned, it may be terrible
-- This is heavily baed on my Godot world generator
GenName = "Debug"
GenApiVersion = 1

function GenerateBlock(x,y,z,blocksSinceSkyVisible)
    local type = 0
    local meta = 0
    if (y == 64) then
        if (x <= 96 and x >= 0) then
            if (z < 16 and z >= 0) then
                    type = x
                    meta = z
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
    return type,meta
end