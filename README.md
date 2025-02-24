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

## Misc
- [PixNBT](https://github.com/OfficialPixelBrush/pixnbt) - A C++ native NBT parsing library
- [BetaPacketPlainTextifier](https://github.com/OfficialPixelBrush/BetaPacketPlainTextifier) - A utility for turning WireShark captures of Minecraft Client-Server data into Markdown files for easy decoding

## Resources
- GitHub
    - [beta-wiki](https://github.com/mudkipdev/beta-wiki) by [mudkipdev](https://github.com/mudkipdev)
- Wiki.vg (Now part of the Minecraft Wiki)
    - [Protocol](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Protocol?oldid=2769758)
    - [Region Files](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Region_Files)
- Minecraft Wiki
    - [NBT Format](https://minecraft.wiki/w/NBT_format)
    - [Region File Format](https://minecraft.wiki/w/Region_file_format)
    - [Help:Isometric Renders](https://minecraft.wiki/w/Help:Isometric_renders) (Used for the Logo)
