GenName = "Sine"
GenApiVersion = 1

function GenerateBlock(x,y,z,blocksSinceSkyVisible)
    type = 0
    if y < 50 + math.sin(x*10)*3 then
        type = getNaturalGrass(x,y,z,blocksSinceSkyVisible)
    end
    return type
end