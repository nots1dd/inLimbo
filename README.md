<div align="center">
  <img src="assets/logo/inLimbo.jpg" alt="Logo">
</div>

<div align="center">
  <h1 style="font-weight: bold; font-size: 3em; color: #3498db;">inLimbo</h1>
</div>

<p align="center">
  <img src="https://img.shields.io/badge/License-GNU GPL%20v3-blue" alt="License" />
  <img src="https://img.shields.io/badge/Version-2.0%20alpha-red" alt="Version" />
  <img src="https://img.shields.io/badge/Build-passing-brightgreen" alt="Build" />
</p>
<p align="center">
  <a href="https://nots1dd.github.io/inLimbo">Documentation</a> ·
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#features">Features</a> ·
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#looks">Looks</a> .
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#building">Get Started</a> ·
  <a href="https://github.com/nots1dd/inLimbo/issues">Report a bug</a>
</p>

> [!WARNING]
> 
> inLimbo is a WIP
> 
> Most aspects of the project are under heavy development
> and no stable release is present as of yet.
> 

The inLimbo project aims to be a new upcoming TUI music player for UNIX based systems that gives music lovers a clean and efficient environment to browse, play and interact with your favourite offline music.

## Features

- **Fully configurable**: Keybinds, library, FTP credentials, and more, all in one `config.toml` file located in `$HOME/.config/inLimbo/config.toml`
- **Very fast and easy to use**: Optimized for speed and usability.
- **Very few external dependencies**: Minimal setup required to get started.
- **Clean and nice TUI experience**: Interact with your music library seamlessly in a terminal interface.
- **Showcases all the metadata**: Displays song metadata, including lyrics, genre, and more, for every file in your library.
- **Plays and caters to many audio file formats**: Compatible with a variety of audio formats, depending on the capabilities of [miniaudio](https://github.com/mackron/miniaudio).
- **Customizable**: Add your own color palette to inLimbo ;)

## LOOKS

<div align="center">
  <img src="assets/gifs/inLimbo-1.gif" alt="inLimbo Demo GIF" width="600">
  <img src="assets/gifs/inLimbo-3.gif" alt="inLimbo Demo GIF" width="600">
</div>

## DEPENDENCIES 

> [!IMPORTANT]
> 
> It is **HIGHLY** recommended to have FFmpeg installed in your system 
> 
> This is required for testing purposes (`./tests/`), and in the future, inLimbo may have features to embed your desired metadata to an audio file. 
> 

| Dependency      | Ubuntu                    | Fedora                        | Arch Linux                   |
|-----------------|---------------------------|-------------------------------|------------------------------|
| **TagLib**      | `libtag1-dev`             | `taglib-devel`                | `taglib`                     |
| **Gio**         | `libgio-2.0-dev`          | `glib2-devel`                 | `glib2`                      |
| **GLib**        | `libglib-2.0-dev`         | `glib2-devel`                 | `glib2`                      |
| **pkg-config**  | `pkg-config`              | `pkgconf`                     | `pkgconf`                    |
| **ImageMagick** | `imagemagick`             | `ImageMagick`                 | `imagemagick`                |
| **X11**         | `libx11-dev`              | `libX11-devel`                | `libx11`                     |
| **PNG**         | `libpng-dev`              | `libpng-devel`                | `libpng`                     |
| **cereal**      | `libcereal-dev`           | `libcereal-devel`             | `cereal`                     |

> [!NOTE]
> 
> If cereal is **NOT** available in your linux distribution,
> 
> ```bash
> git clone https://github.com/USCiLab/cereal.git # download cereal 
> mv cereal/include/* /usr/include/ # moves include/cereal headers to your include directory 
> # If you find that risky, just modify CMakeLists.txt to include cereal's directory for building
> ```
> 

**FTXUI** is fetched from GitHub [here](https://github.com/ArthurSonzogni/FTXUI/) itself during building so **NO** need to install it separately.

## GOAL

The goal of this project is just to find peace and solitude for me as I try to create something I wanted for a long time

Here are a few novelties that may or may not have been added yet:

1. Cool and amazing TUI experience thanks to FTXUI 
2. Full use of metadata (lyrics, genre, thumbnail, etc..) and available for display 
3. Simple and easy network transfer of your favourite songs over to another device
4. Fast and efficient loading of your song directory with everything you need :)
5. TUI over the web?? (Why would you want this)
6. Active testing of the codebase using GTest Suite
7. Security ensured using CodeQL workflow

## **BUILDING**

You will need CMake and Make (GNU Make for BSD) for this (and git obvi)

A simple command using makefile should build everything you want:

```bash 
make build-all
```

Check out [BUILD.md](https://github.com/nots1dd/inLimbo/blob/main/BUILD.md) for more options to build targets

> [!NOTE]
> 
> To try debug build check out [DEBUGGING](https://github.com/nots1dd/inLimbo?tab=readme-ov-file#debugging)
> 

## DEBUGGING

inLimbo is in active development and is prone to having *A LOT* of issues

To try out **DEBUG BUILD** to find bugs/issues:

1. Debug build with `AddressSanitizer` (ASan):

```bash 
mkdir build-dbg/
cd build-dbg/
cmake -DCMAKE_BUILD_TYPE=Debug-ASan ..
make
./inLimbo-DBG-Asan
```

2. Debug build with `Thread Sanitizer` (TSan):

```bash 
mkdir build-dbg/
cd build-dbg/
cmake -DCMAKE_BUILD_TYPE=Debug-TSan ..
make
./inLimbo-Debug-Tsan
```

The **DEBUG BUILD** will account for the following issues:

-> Threads

-> Address Sanitation (ASan)

-> Undefined Behaviour Sanitation (UBSan)

## CONFIGURATION

There is quite a lot to configure in the inLimbo project. A basic config file will be setup in your `$HOME/.config/inLimbo/` directory while using CMake to build

> [!IMPORTANT]
> 
> Check out [config.toml](https://github.com/nots1dd/inLimbo/blob/main/src/parser/examples/config.toml) to see all the possible fields
> for configuration 
> 
> **IT IS UNDER HEAVY DEVELOPMENT SO EXPECT BREAKING CHANGES WITH EACH COMMIT**
>

**COLORS FORMAT**

-> Hexadecimal `#RRGGBB` format 

-> TrueColors (Solid predefined colors):

> [!NOTE]
> 
> For TrueColors, check out [COLORS](https://github.com/nots1dd/inLimbo/blob/main/COLORS.md) for more information on how to set up colors in `inLimbo`
> 

<div align="center">
  <img src="assets/gifs/inLimbo-2.gif" alt="inLimbo Demo GIF" width="600">
</div>

## DIRECTORY SORTING 

To know more about inLimbo Project's logical flow on creating a song map, visit [SONGMAP](https://github.com/nots1dd/inLimbo/blob/main/src/dirsort/SONGMAP.md)

## TESTING 

The inLimbo project is aiming to be more aware of how bad its codebase is, hence I am setting up unit tests for most if not all functions/classes that should:

1. Give more clarity on how the logical flow of the function/class works in a test suite 
2. Logical flaws and boundary conditions going unchecked can be understood better
3. Should give more confidence in the code structure if such tests pass 

The testing is done using GTest, read [TESTS.md](https;//github.com/nots1dd/inLimbo/blob/main/tests/TESTS,md) to understand more on how tests are configured and built

## DOCUMENTATION

There is a primitive Doxygen documention set up for the inLimbo project, that should explain a lot more about the thought process and idea behind every aspect of the codebase.

Man page is also under progress

To generate and view doxygen documentation locally:

```bash 
doxygen .Doxyfile # ensure that you have doxygen installed first 
cp -r assets/gifs/ docs/
cp index.html docs/
firefox docs/html/index.html # or any web browser (xdg-open also works)
```

It is much easier to use the script `make_docs.sh`

```bash
./make_docs.sh
```

Or you could just visit [inLimbo Website](https://nots1dd.github.io/inLimbo/)

## CREDITS

**LOGO**:

*DISCLAIMER*: This is not an original logo nor was it commisioned by me

It is a basic stock image found online. I got it from [here](https://unsplash.com/photos/black-and-white-decor-FrQKfzoTgsw)

**TOML PARSER**:

To parse the [config.toml](https://github.com/nots1dd/inlimbo/blob/main/src/parser/examples/config.toml) is thanks to [TOML++](https://github.com/marzer/tomlplusplus)

**AUDIO PLAYBACK & API**:

Audio playback and capture is thanks to [miniaudio](https://github.com/mackron/miniaudio)

----

inLimbo is Free and Open Source Software Licensed under [GNU GPL v3](https://github.com/nots1dd/inlimbo/blob/main/LICENSE)
