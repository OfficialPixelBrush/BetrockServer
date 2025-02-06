# BetrockServer
A Server-Engine written in C++, made to work with Minecraft Beta 1.7.3. The goal being to have a semi-modern Server Engine that people can muck about with however they like, and to not have to deal with plugins that're over a decade old.
![BetrockServer Logo](media/betrock_server_logo.png)

## Getting Started
How do you run your own BetrockServer instance?

## Option 1 - Download
Check the releases page for the latest binary of BetrockServer. Then simply run the executable.

## Option 2 - Compiling
### Install Dependencies
```bash
sudo apt install libdeflate-dev liblua5.4-dev
```

### Clone
```bash
git clone https://github.com/OfficialPixelBrush/BetrockServer.git
cd BetrockServer
```
```bash
cmake -S . -B build
```

### Build
```bash
cmake --build build
```

### Packing (Optional)
```bash
cpack --config build/CPackConfig.cmake -G TGZ
```
## Progress
- [x] Handshake 
- [x] Login
- [x] Generate a chunk
- [x] Compress the chunk with zlib
- [x] Send the chunk data
- [x] Have a player join
- [x] Fix Client-side crash
- [x] Show Players on other Clients
- [x] Saving and Loading
- [x] Lua World Generation
- [ ] Show other players crouching
- [ ] Add McRegion Support
- [ ] Properly handle inventory

## Resources
- [beta-wiki by mudkipdev](https://github.com/mudkipdev/beta-wiki)
- [wiki.vg (Now part of the Minecraft Wiki)](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Protocol?oldid=2769758)

**Used for the Logo**
- [Help:Isometric Renders (Minecraft Wiki)](https://minecraft.wiki/w/Help:Isometric_renders)