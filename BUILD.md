# inLimbo Build Guide

This project uses a `Makefile` to automate the build process. The `Makefile` supports different build configurations to help with development, testing, and production. Below are the various types of builds you can perform and how to use them.

You can build the inLimbo in two ways:

1. Makefile (uses CMake)
2. Manually building using CMake commands

> [!NOTE]
> 
> If you want to use a custom compiler rather than the default compiler defined in:
> `CMAKE_CXX_COMPILER` macro by CMake,
> 
> Append `CMAKE_EXTRA_FLAGS="-D CMAKE_CXX_COMPILER=/path/to/your/compiler"` to the make command like so:
> 
> ```bash 
> # Let us say I want to compile using clang++:
> make build CMAKE_EXTRA_FLAGS="-D CMAKE_CXX_COMPILER=clang++"
> # To rebuild 
> make rebuild 
> ```
> 
> The CMAKE_EXTRA_FLAGS macro is passable to `build`, `build-all`, `asan`, `build-test`, `build-test-all`, `tsan`.
> 
> There is some issue where clang++ starts a scratch compilation even though the build files are present and only a few source files are changed.
> 

## Available Build Targets

### 1. **all**
This is the default target, and it will perform a full build of the project.

- **Command**: 
  ```sh
  make all
  ```
- **Description**: 
  This will run the `build-all` target, which first executes the initialization script (`init.sh`) and then proceeds with the build.

---

### 2. **build**
This target performs a clean build with the default `Release` configuration.

- **Command**: 
  ```sh
  make build
  ```
- **Description**: 
  - Builds the project using the default `Release` configuration.
  - The build process uses the `CMake` tool and the `build` directory.
  - It ensures that testing is disabled.

---

### 3. **rebuild**
This target will rebuild the project, even if it was previously built.

- **Command**: 
  ```sh
  make rebuild
  ```
- **Description**: 
  - Rebuilds the project by running the `CMake` build process again without needing to clean the build directory.
  - It uses the existing build directory and rebuilds with the same configuration.

---

### 4. **asan (AddressSanitizer)**
This target builds the project with **AddressSanitizer**, which helps detect memory errors.

- **Command**: 
  ```sh
  make asan
  ```
- **Description**: 
  - Builds the project in debug mode with AddressSanitizer enabled.
  - This helps identify memory-related issues such as buffer overflows and memory leaks.

---

### 5. **asan_run**
This target runs the AddressSanitizer build after it has been built.

- **Command**: 
  ```sh
  make asan_run
  ```
- **Description**: 
  - Runs the project with the AddressSanitizer build.
  - Useful for detecting runtime memory issues.

---

### 6. **tsan (ThreadSanitizer)**
This target builds the project with **ThreadSanitizer**, which detects data races in multi-threaded code.

- **Command**: 
  ```sh
  make tsan
  ```
- **Description**: 
  - Builds the project in debug mode with ThreadSanitizer enabled.
  - This helps detect race conditions and other threading issues in the code.

---

### 7. **tsan_run**
This target runs the ThreadSanitizer build after it has been built.

- **Command**: 
  ```sh
  make tsan_run
  ```
- **Description**: 
  - Runs the project with the ThreadSanitizer build.
  - Useful for detecting threading issues during runtime.

---

### 8. **build-test**
This target builds the project with testing enabled. (Also useful for rebuilding tests)

- **Command**: 
  ```sh
  make build-test
  ```
- **Description**: 
  - Builds the project with Google Test (`GTest`) enabled.
  - Useful for running unit tests and validating code functionality.
  - Will **NOT** run the `./init.sh` script

---

### 9. **build-test-all**
This target runs the initialization script and builds the project with testing enabled.

- **Command**: 
  ```sh
  make build-test-all
  ```
- **Description**: 
  - Runs the `init.sh` script and then builds the project with testing enabled.
  - The `build-test` target is triggered after the script execution.

---

### 10. **clean**
This target removes all build artifacts, effectively cleaning the project.

- **Command**: 
  ```sh
  make clean
  ```
- **Description**: 
  - Deletes the build directory and all associated files, allowing for a fresh build.

---

### 11. **init**
This target runs the initialization script (`init.sh`).

- **Command**: 
  ```sh
  make init
  ```
- **Description**: 
  - Executes the initialization script to set up any necessary environment or configuration before building.

---

### 12. **build-global**
This target builds and installs the project globally.

- **Command**: 
  ```sh
  make build-global
  ```
- **Description**: 
  - Just runs sudo make install after building for target local release (adds inLimbo.desktop and icon to system wide directories)

---

### 12. **build-global-uninstall**
This target uninstalls every system wide installed file of inLimbo using `install_manifests.txt` in build directory.

- **Command**: 
  ```sh
  make build-global-uninstall
  ```

  > [!NOTE]
  > 
  > If you remove `build/` directory, the `install_manifests.txt` will not exist so this wont work
  > 
  > Will come with a workaround in the future
  > 

---

### 13. **verbose**
This target enables verbose output during the build process.

- **Command**: 
  ```sh
  make verbose
  ```
- **Description**: 
  - Builds the project with verbose output to give more detailed information about the build process.
  - This is helpful for debugging build issues.

---

## **MANUAL BUILD**:

> [!WARNING]
> 
> Proceed with manual build **ONLY** if you know what you are doing.
> 

```bash
git clone https://github.com/nots1dd/inLimbo.git 
cd inLimbo/
./init.sh # to initialize the repository with some important headers
```

Then building commands: (**WILL NOT COMPILE IF `init.sh` IS NOT RUN FIRST**)

```bash
cmake -S . -B build/
cmake --build build/
./build/inLimbo
```

To build inLimbo **GLOBALLY**:

```bash 
cmake -S . -B build -DGLOBAL_BUILD=ON 
cmake --build build/ 
cd build 
sudo make install # will put the binary in /usr/bin and respective inLimbo.desktop and logo in /usr/share
```

To build its web-assembly, you will need [emscripten](https://github.com/emscripten-core/emscripten)

> [!IMPORTANT]
> 
> Building the web-assembly:
> 
> Currently I am not sure how to include and compile taglib
> into wasm so currently the web build may compile but it will 
> not work as I have dummy methods set up for EMSCRIPTEN build 
> (else it wont even compile to wasm)
> 

```bash 
mkdir build_emscripten && cd build_emscripten 
emcmake cmake ..
make -j
./run_webassembly.py # assuming the compilation has no errors
# This will run in port 8000
```
