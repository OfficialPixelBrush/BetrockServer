# BetrockServer
 A Server-Engine written in C++, made to work with Minecraft Beta 1.7.3

- [x] Handshake 
- [x] Login
- [ ] Generate a chunk
- [ ] Compress the chunk with zlib
- [ ] Send the chunk data
- [x] Have a player join

## Installing Dependencies
```bash
sudo apt install libdeflate-dev
```

## Building
```bash
cmake -B build -S.
cmake --build build --config Debug --target all
```