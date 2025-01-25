# BetrockServer
 A Server-Engine written in C++, made to work with Minecraft Beta 1.7.3

- [x] Handshake 
- [x] Login
- [x] Generate a chunk
- [x] Compress the chunk with zlib
- [x] Send the chunk data
- [x] Have a player join
- [x] Fix Client-side crash
- [x] Show Players on other Clients
- [x] Saving and Loading
- [ ] Show other players crouching
- [ ] Add McRegion
- [ ] Properly handle inventory

## Getting Started
How do you run your own BetrockServer instance?

## Option 1 - Download
Check the releases page for the latest binary of BetrockServer. Then simply run the executable.

## Option 2 - Builing
### Cloning
```bash
git clone https://github.com/OfficialPixelBrush/BetrockServer.git
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

### Installing Dependencies
```bash
sudo apt install libdeflate-dev liblua5.4-dev
```

### Building
```bash
cmake --build build
```

### Packing (Optional)
```bash
cpack --config build/CPackConfig.cmake -G TGZ
```

## Resources
- [beta-wiki](https://github.com/mudkipdev/beta-wiki)
- [wiki.vg (Now part of the Minecraft Wiki)](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Protocol?oldid=2769758)