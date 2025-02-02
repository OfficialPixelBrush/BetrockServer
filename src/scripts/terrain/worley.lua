-- Up ahead is my own attempt at a world generator.
-- Be warned, it may be terrible
-- This is heavily baed on my Godot world generator

function GenerateBlock(x,y,z,blocksSinceSkyVisible)
    local type = 0
    local solid = true
    if (y == 0) then
        type = 7
    elseif y > 0 then
        if (between(y,0,3) and spatialPrng(x,y,z)%2) then
            type = 7
            return type
        end
        if (getNoiseWorley(x,128-y-13,z,60,0.1) > 0.2) then
            solid = false
        end

        if (solid) then
            type = getNaturalGrass(x,y,z,blocksSinceSkyVisible)
        elseif (y < 64) then
            type = 9
        end
    end
    return type
end