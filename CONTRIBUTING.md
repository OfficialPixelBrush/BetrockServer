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

## Build
Lastly, build with the `Debug` config to make debugging easier.
We use `-fsanitize=address` to provide us with a readable debug trace.
```bash
cmake --build . --config Debug
```
