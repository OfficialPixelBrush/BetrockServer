import zlib
Size_X = 15
Size_Y = 127
Size_Z = 15
GroundLevel = 50
maxSize = (Size_X+1) * (Size_Y+1) * (Size_Z+1) * 2.5

data = []

#for i in range(int((Size_X+1) * (Size_Y+1) * (Size_Z+1) * 2.5)):
#    data.append(1)

for x in range(Size_X+1):
    for z in range(Size_Z+1):
        for y in range(Size_Y+1):
            if (y <= GroundLevel):
                data.append(1)
            else:
                data.append(0)

# Block Metadata
for x in range(Size_X+1):
    for z in range(Size_Z+1):
        for y in range(Size_Y+1):
            if (y%2):
                data.append(0)
            else:
                data[-1] = (data[-1] << 4) | 0

# Block Light
for x in range(Size_X+1):
    for z in range(Size_Z+1):
        for y in range(Size_Y+1):
            if (y%2):
                data.append(0)
            else:
                data[-1] = (data[-1] << 4) | 0

# Sky Light
for x in range(Size_X+1):
    for z in range(Size_Z+1):
        for y in range(Size_Y+1):
            skyLight = 15
            if (y <= GroundLevel):
                skyLight = 0
            
            if (y%2):
                data.append(skyLight)
            else:
                data[-1] = (data[-1] << 4) | skyLight

# Compress the data
compressed_data = zlib.compress(bytes(data), level=zlib.Z_BEST_COMPRESSION)

# Write the compressed data to the output file
with open("chunk.txt", 'w') as f:
    for b in compressed_data:
        f.write(str(int(b)) + ",")