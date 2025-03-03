# BetrockServer
A Server-Engine written in C++, made to work with Minecraft Beta 1.7.3. The goal being to have a semi-modern Server Engine that people can muck about with however they like, and to not have to deal with plugins that're over a decade old.
![BetrockServer Logo](media/betrock_server_logo.png)

## Features
(as of 0.2.0)
- Players can connect, chat, build and explore
- Commands
- Lua-driven World Generation

## Getting Started
How do you run your own BetrockServer instance?

## Option 1 - Download
Check the releases page for the latest binary of BetrockServer. Then simply run the executable.

If it complains about lacking a `scripts` folder, download the `scripts` folder from the repository and place it alongside the executable.

## Option 2 - Compiling
This section is written to be as accessible as possible, so anyone can compile BetrockServer.

### Install Dependencies
```bash
sudo apt install build-essential git libdeflate-dev liblua5.4-dev
```
This installs all the necessary libraries that BetrockServer needs.

### Clone
```bash
git clone --recurse-submodules https://github.com/OfficialPixelBrush/BetrockServer.git
cd BetrockServer
```
This downloads the repository onto your computer.
```bash
cmake -S . -B build
cd build
```
This configures the development environment.

### Build
```bash
cmake --build .
```
This turns the source code into an executable binary file.

### Running
```bash
./BetrockServer
```
This runs the built executable and generates a new world,
if one isn't already present.

### Packing (For Releasing)
```bash
cpack --config CPackConfig.cmake -G TGZ
```

## Misc
- [PixNBT](https://github.com/OfficialPixelBrush/pixnbt) - A C++ native NBT parsing library
- [BetaPacketPlainTextifier](https://github.com/OfficialPixelBrush/BetaPacketPlainTextifier) - A utility for turning WireShark captures of Minecraft Client-Server data into Markdown files for easy decoding

# Contributing
Simply fork the [`latest` branch](https://github.com/OfficialPixelBrush/BetrockServer/tree/latest) and commit whatever changes you want to make.

Please, rebase to the latest commit and squash all your changes into a single commit before making a PR. This just makes it easier to merge into the branch.

The [SerenityOS](https://github.com/SerenityOS/serenity) folks do this as well :p

## Issues
If you encounter **any** issues, inaccuracies to the base game or simply want to provide an idea that may make BetrockServer better,
please report them on the [Issues tab](https://github.com/OfficialPixelBrush/BetrockServer/issues).

Please check if the issue you've found has already been reported or even been solved already.

If not, create a **new issue** and fill out all the necessary information according to the `Bug Report` template to the best of your abilities.

## Development
Grab the `latest` branch for the most up-to-date, albeit unstable, repository.
```bash
git pull
git branch -r
git checkout origin/latest
```
If any of the submodules have been updated, grab their latest version.
```bash
git submodule update --recursive --remote
```

### Build
Lastly, build with the `Debug` config to make debugging easier.
We use `-fsanitize=address` to provide us with a readable debug trace.
```bash
cmake --build . --config Debug
```

# Resources
For further reading, to aid development and sources for where certain info has been gotten from.

Some stuff can be found on the [BetrockServer Wiki](https://github.com/OfficialPixelBrush/BetrockServer/wiki),
especially if you're interested in developing your own [World Generator](https://github.com/OfficialPixelBrush/BetrockServer/wiki/World-Generation) or [Plugin](https://github.com/OfficialPixelBrush/BetrockServer/wiki/Plugins). It also features a very complete [ID Listing](https://github.com/OfficialPixelBrush/BetrockServer/wiki/Full-ID-Listing),
though the one hosted on [grahamedgecombe.com](https://minecraft-ids.grahamedgecombe.com/) is a great alternative for general usage.

- GitHub
    - [beta-wiki](https://github.com/mudkipdev/beta-wiki) by [mudkipdev](https://github.com/mudkipdev)
- Wiki.vg (Now part of the Minecraft Wiki)
    - [Protocol](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Protocol?oldid=2769758)
    - [Region Files](https://minecraft.wiki/w/Minecraft_Wiki:Projects/wiki.vg_merge/Region_Files)
- Minecraft Wiki
    - [NBT Format](https://minecraft.wiki/w/NBT_format)
    - [Region File Format](https://minecraft.wiki/w/Region_file_format)
    - [Help:Isometric Renders](https://minecraft.wiki/w/Help:Isometric_renders) (Used for the Logo)

# Stats
![Contributors](https://img.shields.io/github/contributors/OfficialPixelBrush/BetrockServer)

| Issues | PRs |
| - | - |
| ![Open Issues](https://img.shields.io/github/issues/OfficialPixelBrush/BetrockServer) | ![Open PRs](https://img.shields.io/github/issues-pr/OfficialPixelBrush/BetrockServer) |
| ![Closed Issues](https://img.shields.io/github/issues-closed/OfficialPixelBrush/BetrockServer) | ![Closed PRs](https://img.shields.io/github/issues-pr-closed/OfficialPixelBrush/BetrockServer) |

| Branch | Commits |
| - | - |
| main | ![Commits (main)](https://img.shields.io/github/commit-activity/t/OfficialPixelBrush/BetrockServer/main) |
| latest | ![Commits (latest)](https://img.shields.io/github/commit-activity/t/OfficialPixelBrush/BetrockServer/latest) |