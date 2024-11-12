# Incolata

This is a very hacky proof of concept, and entirely unvalidated, fit-testing
application. It's expected to work with any PortaCount 8020 and 8020A, however
it's only been tested with a single 8020A.

> [!WARNING]
> This codebase is extremely messy.

![Main Window, showing a test in progress](/docs/img/screenshot_main_2024_11_02.png?raw=true "Screenshot of the test window")

## Setup guide

### Linux

**Dependencies:**

* Rust+Cargo
* Qt6, Qt6-Charts ('qt6-charts-dev' or equivalent should pull in all the right Qt deps)
* CMake

**Nice-to-have:**

* Ninja (e.g. 'ninja-build') - the example build instructions below use ninja,
  but you can also tell CMake to build using another tool if you prefer.

**Build it:**

See also the Github Actions Workflow at:
https://github.com/ahunt/incolata/blob/main/.github/workflows/checks.yml

I typically run something like:
```
git submodule init
cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S . -B build/Debug
cd build/Debug && ninja
```

> [!TIP]  
> You can also open the project in QtCreator, which should take care of
> configuring and building, modulo submodule init.

### Windows

TODO

### OSX

TODO, if I ever get hold of a Mac.

## Architecture

Actual test machinery will live in libp8020a. Incolata is just a (not so) nice
(yet) GUI wrapper around that.

There is no real architecture to speak of, yet.
