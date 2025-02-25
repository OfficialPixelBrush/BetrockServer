GenName = "Perlin"
GenApiVersion = 1

-- Fade function for smooth interpolation
local function fade(t)
    return t * t * t * (t * (t * 6 - 15) + 10)
end

-- Linear interpolation
local function lerp(a, b, t)
    return a + t * (b - a)
end

-- Gradient function for generating pseudo-random gradients
local function grad(hash, x, y)
    local h = hash % 4
    if h == 0 then return x + y end
    if h == 1 then return x - y end
    if h == 2 then return -x + y end
    return -x - y
end

-- XOR-Shift PRNG (64-bit)
local function xorShift(seed)
    seed = seed ~ (seed << 13)
    seed = seed ~ (seed >> 7)
    seed = seed ~ (seed << 17)
    return seed & 0xFFFFFFFFFFFFFFFF  -- Ensure 64-bit result
end

-- Generate permutation table with external seed
local p = {}
local function initializePermutation(seed)
    local temp = {}
    for i = 0, 255 do
        temp[i] = i
    end

    -- Shuffle using XOR-Shift PRNG
    for i = 255, 1, -1 do
        seed = xorShift(seed)
        local j = seed % (i + 1)
        temp[i], temp[j] = temp[j], temp[i]
    end

    -- Fill permutation table and duplicate it for safe lookup
    for i = 0, 255 do
        p[i] = temp[i]
        p[i + 256] = temp[i]
    end
end

-- Call permutation initialization with the external seed
initializePermutation(seed)

-- Perlin noise function
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

-- Terrain generation using seeded Perlin noise
function GenerateChunk(cx,cz)
    local c = {}
    for x = 0, CHUNK_WIDTH_X do
        for z = 0, CHUNK_WIDTH_Z do
            for y = 0, CHUNK_HEIGHT do
                local type = 0
                if y < 40 + ((perlin((cx*16+x) / 64, (cz*16+z) / 64) + 1) * 25) then
                    type = 3 --getNaturalGrass(x, y, z, blocksSinceSkyVisible)
                end
                if y < 64 and type == 0 then
                    type = 9 -- Water
                end
                c[index(x, y, z)] = {type, 0}
            end
        end
    end
    return c
end