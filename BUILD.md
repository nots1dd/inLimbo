# inLimbo Build Guide

This document explains how to build, test, format, and maintain the **inLimbo** project using the provided **Makefile**.
The Makefile is a thin, explicit wrapper around **CMake**. 

> [!IMPORTANT]
> 
> This doc maybe outdated or contradictory at times!!
> 
> It is **highly advised** to run `make help`
> for the latest targets and what they do 
> in the Makefile itself.
> 

## Requirements

Before building, ensure the following tools are available:

> [!NOTE]
>
> Although the CMake build allows versions starting from `3.5`,
> this is **not actively tested**.
> It is strongly recommended to use the **latest CMake version**
> available for your distribution.

* CMake (3.16 or newer recommended)
* CTest (comes with CMake)
* A C++20-compatible compiler (clang or gcc)
* make
* clang-format (for formatting)
* clang-tidy (for static analysis)
* Git (for submodules)

You can verify required tools using:

```bash
make verify-deps
```

If dependencies are missing, the Makefile provides a helper to install them using your system package manager:

```bash
make install-deps
```

Supported distributions:

* Arch Linux
* Debian / Ubuntu
* Fedora

## Build Overview

The project supports **Release** and **Debug** builds, each in its own build directory:

* Release build: `build/`
* Debug build: `build-dbg/`

The Makefile separates **configuration** (CMake generation) from **building**, and additionally supports **selectable frontends**.

## Frontend Selection

inLimbo supports multiple frontends (currently **cmdline**, with more planned).
Frontend selection is handled via a Makefile variable that forwards to CMake:

```text
INLIMBO_FRONTEND_TYPE
```

### Default frontend

If no frontend is specified, the build **defaults to `cmdline`**.

## Building with a Frontend (Recommended)

### Release build with frontend

```bash
make app
```

This defaults to:

```text
frontend = cmdline
```

To explicitly select a frontend:

```bash
make app name=cmdline
```

Internally, this runs:

```bash
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release -DINLIMBO_FRONTEND_TYPE=cmdline
cmake --build build
```

### Debug build with frontend

```bash
make app-dbg
```

Or explicitly:

```bash
make app-dbg name=cmdline
```

This additionally ensures **libbacktrace** is built and linked in Debug mode.

## Building Without Frontend Helpers (Direct CMake Flow)

You can still use the raw build targets if you prefer direct control.

> [!NOTE]
> 
> You can pass custom cmake flags via `EXTRA_CMAKE_FLAGS`
> in Makefile like so:
> 
> ```bash
> make <cmd> EXTRA_CMAKE_FLAGS="-D INLIMBO_ENABLE_TESTING=OFF"
> ```
>
> Here `<cmd>` can be `buildx` or `buildx-dbg` (it initializes CMake)
> with the required flags.
> 
> As such, it is obvious that rebuilds (like `build` or `build-dbg` targets)
> will not take these flags properly.
> 

### Release build

#### First-time build (configure + build)

```bash
make buildx
```

Equivalent to:

```bash
cmake -S . -B build $(EXTRA_CMAKE_FLAGS) -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

#### Incremental build

```bash
make build
```

### Debug build

#### First-time debug build

```bash
make buildx-dbg
```

Equivalent to:

```bash
cmake -S . -B build-dbg $(EXTRA_CMAKE_FLAGS) -D CMAKE_BUILD_TYPE=Debug
cmake --build build-dbg
```

#### Incremental debug build

```bash
make build-dbg
```

### Rebuilding from scratch

```bash
make rebuild
```

Debug:

```bash
make rebuild-dbg
```

## Running Tests

Tests are built and executed using **GoogleTest** and **CTest**.

### Run tests (Release)

```bash
make test
```

Equivalent to:

```bash
ctest --test-dir build --output-on-failure
```

### Run tests (Debug)

```bash
make test-dbg
```

Equivalent to:

```bash
ctest --test-dir build-dbg --output-on-failure
```

## Formatting and Static Analysis

### Format all source files

```bash
make fmt
```

### Check formatting (CI-safe)

```bash
make fmt-check
```

### Run clang-tidy

```bash
make tidy
```

Requires `compile_commands.json`.
If missing:

```bash
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build
```

## Git Submodules

### Initialize submodules

```bash
make submod-init
```

### Verify submodules

```bash
make submod-check
```

## Cleaning Build Artifacts

```bash
make clean
```

Removes:

* `build/`
* `build-dbg/`



## Project Statistics

```bash
make stats
```

Displays:

* number of source files
* number of header files
* total tracked files

## Run All Checks

For a full local verification (recommended before pushing):

```bash
make all-checks
```

Runs:

* dependency verification
* formatting checks
* clang-tidy analysis

## Help

To list all available Makefile targets:

```bash
make help
```

## Recommended Workflow

### Day-to-day development

```bash
make app              # configure + build with default frontend
make build            # incremental builds
make test             # run tests
make fmt              # format code
```

### Debugging

```bash
make app-dbg
make test-dbg
```

## Notes

* The Makefile **does not replace CMake**; it standardizes common workflows.
* Frontends are **modular** and can be built or replaced independently.
* Release and Debug builds are fully isolated.
* All targets are explicit — no hidden behavior.
* Future frontends can be added without changing the core build logic.
