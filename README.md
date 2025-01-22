# BetrockServer
 A Server-Engine written in C++, made to work with Minecraft Beta 1.7.3

- [x] Handshake 
- [x] Login
- [x] Generate a chunk
- [x] Compress the chunk with zlib
- [x] Send the chunk data
- [x] Have a player join
- [ ] Fix Client-side crash
- [ ] Show Players on other Clients

## Installing Dependencies
```bash
sudo apt install libdeflate-dev
```

## Building
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Resources
- [beta-wiki](https://github.com/mudkipdev/beta-wiki)
- [wiki.vg (Now part of the Minecraft Wiki)](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Protocol?oldid=2769758)