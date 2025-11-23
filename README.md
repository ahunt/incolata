# Incolata

This is a very hacky proof of concept, and entirely unvalidated, fit-testing
application.

![Main Window, showing a test in progress](/docs/img/screenshot_main_2025_05_03.png?raw=true "Screenshot of the test window")

## Supported

* Devices:
    * 8020 (non-A).
    * 8020A.
    * 8020Mgen1 (aka M41 PATS).
    * 8020Mgen2 (aka M41A1 PATS) - _only via serial port (not via USB)_.
* Protocols: OSHA classic and modified (fast), Crash2.5, ISO, and HSE.
    * Other protocols can be easily added but (for the time being) require
      recompilation.
* Platforms: Linux, MacOS.
* CSV logs containing per-exercise FFs.

## Planned

* Dual-device support (e.g. concurrent total particle count + leak integrity
  measurement).
* Raw data logging.
* Windows support.
* Custom protocol support (without having to edit sources, followed by
  recompiling).
* 8095 support. Its already possible to run tests with an 8095, but this isn't
  recorded anywhere (ideally logs would be annotated somehow).

### Not planned

* Support for 803X and 804X. There are too few of these in the community to
  justify the effort (at least for now).
* Support for non-TSI CPCs, except for perhaps the OpenCPC/OpenAeros if/when I
  get a hold of one, and figure out how to rig up a switching valve.

## Docs

* [8020 resources, & hints & tips](docs/8020.md).

## Development guide

### Linux

**Dependencies:**

* Rust+Cargo
* Qt6, Qt6-Charts ('qt6-charts-dev' or equivalent should pull in all the right Qt deps)
* CMake
* GCC or other C++ compiler

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

#### Static Analysis

(Requires installing clang.)

```
scan-build cmake . -B build/analysis
cd build/analysis && scan-build make
```

### Windows

TODO

> [!NOTE]
> Some, but not all, Serial->USB adapters work out of the box. Here it's the
> opposite of the OSX situation: my Aten (with Prolific chip) just works, and my
> FTDI FT232R-based adapter required me to install [their driver](https://ftdichip.com/drivers/).

### OSX

1. Install [Homebrew](https://brew.sh)

2.
   ```
   brew install git rust cmake qt ninja
   git clone https://github.com/ahunt/incolata.git && cd incolata &&  && git submodule init & git submodule update
   cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S . -B build/Debug
   cd build/Debug && ninja && open incolata.app
   ```

> [!NOTE]
> Some, but not all, Serial->USB adapters work out of the OSX. My (possibly
> fake) FTDI FT232R-based adapter does work, whereas my Aten (with Prolific
> chip) requires [installing a Driver](https://www.aten.com/global/en/supportcenter/info/downloads/?action=display_product&pid=575).
> (Don't forget to unblock the driver via System Settings->Privacy &
> Security->Enable System Extensions after installation.) Prolific offer a
> [PL2303 Serial Driver on the app store](https://apps.apple.com/us/app/pl2303-serial/id1624835354?mt=12),
> which might also work for the same Aten adapter, but I have not tested it.

## Architecture

Actual test machinery will live in libp8020a. Incolata is just a (not so) nice
(yet) GUI wrapper around that.

There is no real architecture to speak of, yet.
