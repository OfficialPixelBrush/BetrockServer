GenName = "Superflat"
GenApiVersion = 1

function GenerateBlock(x,y,z,blocksSinceSkyVisible)
    type = 0
    if y == 0 then
        type = 7
    elseif y > 0 and y < 3 then
        type = 3
    elseif y == 3 then
        type = 2
    end
    return type
end