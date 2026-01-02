<div align="center">
  <img src="assets/logo/inLimbo.jpg" alt="Logo">
</div>

<div align="center">
  <h1 style="font-weight: bold; font-size: 3em; color: #3498db;">inLimbo</h1>
</div>

<p align="center">
  <img src="https://img.shields.io/badge/License-GNU GPL%20v3-blue" alt="License" />
  <img src="https://img.shields.io/badge/Version-3.0%20alpha-red" alt="Version" />
  <img src="https://img.shields.io/badge/Build-passing-brightgreen" alt="Build" />
</p>
<p align="center">
  <a href="https://nots1dd.github.io/inLimbo">Documentation</a> ·
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#features">Features</a> ·
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#looks">Looks</a> .
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#building">Get Started</a> ·
  <a href="https://github.com/nots1dd/inLimbo/issues">Report a bug</a>
</p>

**MOST OF THIS README IS OUTDATED OR IRRELEVANT TO THIS BRANCH.**

> [!CAUTION]
> 
> This is the develop branch! 
> 
> Firstly ensure that you have **ALL** the dependencies resolved.
> (**spdlog**, **toml++**, **miniaudio** and **cereal** should all be resolved by git and cmake itself!)
> 
> You will have to install **CMake**, **Git**, **C++20 Compiler (GCC/Clang/MSVC)**, **libbacktrace**!!
> 
> Have the repo cloned **WITH THE SUBMODULES**:
> 
> ```bash
> # clone and load submodules 
> git clone https://github.com/nots1dd/inlimbo 
> cd inlimbo 
> git checkout develop
> git submodule update --init --recursive
> 
> # or 
> # clone repo with recursive SUBMODULES directly
> git clone -b develop https://github.com/nots1dd/inlimbo --recursive
> 
> make buildx
> ```
> 

> [!WARNING]
> 
> inLimbo is a WIP
> 
> Most aspects of the project are under heavy development
> and no stable release is present as of yet.
> 

The inLimbo project aims to be a new upcoming TUI music player for UNIX based systems that gives music lovers a clean and efficient environment to browse, play and interact with your favourite offline music.

## **Why**

There are a dozen of great music players out there that do a really good job of making my music experience top notch, but nothing was ever perfect. So I tried making something that in my opinion comes closest to my ideal TUI music player.

Why would you want to stay in Limbo?

Sometimes it feels good to be stuck in a weird place in your life, and it feels even better to be stuck in Limbo with music :)

## **DEPENDENCIES** 

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

## **BUILDING**

> [!IMPORTANT]
> 
> inLimbo has been **TRIED AND TESTED** in the following GNU/Linux distros:
> 
> 1. Arch Linux (x86-64 arch)
> 2. Ubuntu (x86-64 arch)
> 3. Fedora/Nobara Linux (x86-64 arch)
> 
> As long as your distribution can install & build the requirements, there should *NOT* be any issue
> 

Check out [BUILD.md](https://github.com/nots1dd/inLimbo/blob/main/BUILD.md) for more options to build targets

## CREDITS

**LOGO**:

*DISCLAIMER*: This is not an original logo nor was it commisioned by me

It is a basic stock image found online. I got it from [here](https://unsplash.com/photos/black-and-white-decor-FrQKfzoTgsw)

**TOML PARSER: TOML++**:

To parse the [config.toml](https://github.com/nots1dd/inlimbo/blob/main/src/parser/examples/config.toml) is thanks to [TOML++](https://github.com/marzer/tomlplusplus)

**Command Line Parsing (Arguments): CXXOPTS**:

To parse the command line arguments (and sparing me time to work on the actual project) is thanks to [cxxopts](https://github.com/jarro2783/cxxopts)

**Serialization: Cereal**:

Easy and fast serialization and deserialization of song map is thanks to [cereal](https://github.com/USCiLab/cereal/)

**Logging: spdlog**:

Modular and fast logging is thanks to [spdlog](https://github.com/gabime/spdlog)

**INSPIRATIONS**:

This music player's UI flow is heavily inspired by [CMUS](https://github.com/cmus/cmus), it seemed (to me atleast) the fastest and most responsive and logical UI/UX design of a TUI that has a great balance between looking good and feeling smooth

inLimbo is Free and Open Source Software Licensed under [GNU GPL v3](https://github.com/nots1dd/inlimbo/blob/main/LICENSE)
