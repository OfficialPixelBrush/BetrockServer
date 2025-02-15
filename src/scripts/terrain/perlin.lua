GenName = "Perlin"
GenApiVersion = 1

local function fade(t)
    return t * t * t * (t * (t * 6 - 15) + 10)
end

local function lerp(a, b, t)
    return a + t * (b - a)
end

local function grad(hash, x, y)
    local h = hash % 4
    if h == 0 then return x + y end
    if h == 1 then return x - y end
    if h == 2 then return -x + y end
    return -x - y
end

local p = {}
for i = 0, 255 do p[i] = i end
for i = 0, 255 do
    local j = math.random(256) - 1
    p[i], p[j] = p[j], p[i]
end
for i = 0, 255 do
    p[i + 256] = p[i]
end

local function perlin(x, y)
    local X, Y = math.floor(x) % 256, math.floor(y) % 256
    x, y = x - math.floor(x), y - math.floor(y)

    local u, v = fade(x), fade(y)
    local a, b = p[X] + Y, p[X + 1] + Y

    return lerp(
        lerp(grad(p[a], x, y), grad(p[a + 1], x, y - 1), v),
        lerp(grad(p[b], x - 1, y), grad(p[b + 1], x - 1, y - 1), v),
        u
    )
end

function GenerateBlock(x,y,z,blocksSinceSkyVisible)
    local type = 0
    local solid = 0
    if y < 40+((perlin(x/64, z/64)+1)*25) then
        type = getNaturalGrass(x,y,z,blocksSinceSkyVisible)
    end
    -- Check if non-solid, generate water
    if (y < 64 and type == 0) then
        type = 9
    end
    return type
end