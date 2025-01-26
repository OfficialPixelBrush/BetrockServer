function naturalGrass(bs)
    local type = 2
    if between(bs,1,3) then
        type = 3
    elseif bs >= 3 then
        type = 1
    end
    return type
end

function GenerateBlock(x,y,z,blocksSinceSkyVisible)
    type = 0
    if y < 50 + math.sin(x*10)*3 then
        type = naturalGrass(blocksSinceSkyVisible)
    end
    return type
end