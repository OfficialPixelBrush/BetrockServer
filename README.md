# BetrockServer
A Server-Engine written in C++, made to work with Minecraft Beta 1.7.3. The goal being to have a semi-modern Server Engine that people can muck about with however they like, and to not have to deal with plugins that're over a decade old.
![BetrockServer Logo](media/betrock_server_logo.png)

## Features
(as of 0.1.14)
- Players can connect, chat, build and explore
- Commands
- Lua-driven World Generation

## Getting Started
How do you run your own BetrockServer instance?

## Option 1 - Download
Check the releases page for the latest binary of BetrockServer. Then simply run the executable.

## Option 2 - Compiling
### Install Dependencies
```bash
sudo apt install build-essential git libdeflate-dev liblua5.4-dev
```

### Clone
```bash
git clone --recurse-submodules https://github.com/OfficialPixelBrush/BetrockServer.git
cd BetrockServer
```
```bash
cmake -S . -B build
cd build
```

### Build
```bash
cmake --build .
```

### Packing (Optional)
```bash
cpack --config CPackConfig.cmake -G TGZ
```

## Resources
- [beta-wiki by mudkipdev](https://github.com/mudkipdev/beta-wiki)
- [wiki.vg (Now part of the Minecraft Wiki)](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Protocol?oldid=2769758)

**Used for the Logo**
- [Help:Isometric Renders (Minecraft Wiki)](https://minecraft.wiki/w/Help:Isometric_renders)
