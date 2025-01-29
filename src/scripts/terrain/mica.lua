-- Based on glitched terrain gen I got while remaking the worley generator in lua

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
        if (getNoiseWorley(x*16,128-y-13,z,60,0.1) > 0.2) then
            solid = false
        end
    end
    if (solid) then
        value = spatialPrng(x,y,z)%4;
        if (value == 0) then
            type = 4
        else
            type = 1
        end
    elseif (y < 64) then
        type = 9
    end
    return type
end