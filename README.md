# inLimbo

A possible revamp of LiteMus

## Goal

The goal of this project is just to find peace and solitude for me as I try to create something I wanted for a long time

Here are a few novelties that may or may not have been added yet:

1. Cool and amazing TUI experience thanks to FTXUI 
2. Full use of metadata (lyrics, genre, etc..) and available for display 
3. Simple and easy network transfer of your favourite songs over to another device
4. Fast and efficient loading of your song directory with everything you need :)
5. TUI over the web?? (Why would you want this)

## Building

You will need CMake for this (and git obvi)

```bash
cmake -S . -B build/
cmake --build build/
./build/inLimbo
```

To build its web-assembly, you will need [emscripten](https://github.com/emscripten-core/emscripten)

> [!IMPORTANT]
> 
> Building the web-assembly:
> 
> It is not perfect and maybe not work
> 

```bash 
mkdir build_emscripten && cd build_emscripten 
emcmake cmake ..
make -j
./run_webassembly.py # assuming the compilation has no errors
# This will run in port 8000
```

## Dirsort 

Currently uses rbtrees and unordered_map to store and write the current inodes in given directory to `lib.sync` file 

-> rbtree will contain all the inodes at runtime

-> cached to `lib.sync` for future use and faster data fetch

> [!NOTE]
> 
> src/dirsort/ has the current implementation of
> how the chosen songs directory will be cached and 
> read into memory to display in ftxui TUI
> 
> To use dirsort/ source files separately:
> 
> ```bash 
> make
> ```
> 
