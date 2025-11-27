# Contributing
Simply fork the [`latest` branch](https://github.com/OfficialPixelBrush/BetrockServer/tree/latest) and commit whatever changes you want to make.

Please, rebase to the latest commit and squash all your changes into a single commit before making a PR. This just makes it easier to merge into the branch.

The [SerenityOS](https://github.com/SerenityOS/serenity) folks do this as well :p

## Development
Grab the `latest` branch for the most up-to-date, albeit unstable, repository.
```bash
git fetch
git branch -r
git checkout -b latest origin/latest
```
If any of the submodules have been updated, grab their latest version.
```bash
git submodule update --recursive --remote
```

## Build
Lastly, build with the `Debug` config to make debugging easier.
We use `-fsanitize=address` to provide us with a readable debug trace.
```bash
cmake --build . --config Debug
```

## Recommended VSCode Extensions
- clangd
- CMake Tools
- Doxygen Documentation Generator

## Style Guide
- If possible, do not pass values separately. Make use of structs that combine them
    - i.e. instead of `int posX, int posY, int posZ`, just use `Int3 pos`
- Run `clang-format -i` over the files you changed