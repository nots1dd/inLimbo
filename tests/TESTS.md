# TESTING

This directory in the inLimbo project source tree is dedicated to performing unit tests on all classes and functionalities using the GTest Suite.

## Why Testing is Essential

Unit testing in the inLimbo project is still in its infancy but it will only grow bigger and better. For the inLimbo project, unit tests:

- **Ensure Code Reliability:** By testing individual functions and classes, we can catch bugs early and verify that they perform as expected.
- **Promote Code Safety:** Testing safeguards against regressions and ensures that changes or updates do not break existing functionality.
- **Improve Maintainability:** With comprehensive tests, developers can confidently refactor code knowing that tests will catch unintended side effects.
- **Document Functionality:** Tests provide living documentation for how functions and classes are expected to behave.
- **Facilitate Collaboration:** Consistent test results make it easier for team members to collaborate and contribute to the project.

## Testing in the inLimbo Project

For the inLimbo project, tests are organized to cover key components of the software, including:

1. **Core Functionality:** Verifying the correctness of algorithms and core logic.
2. **Class Behavior:** Ensuring individual classes behave as designed under different scenarios.
3. **Integration Points:** Testing interactions between components to ensure seamless integration.
4. **Edge Cases:** Handling unexpected or extreme inputs gracefully without crashing.

## Prerequisites

Ensure that GoogleTest is installed on your Linux distribution. You can typically install it via your package manager:

**For Ubuntu (Debian):**

```bash
sudo apt install libgtest-dev cmake # For Ubuntu/Debian systems
```

**For Fedora (RPM):**

```bash 
sudo dnf install gtest-devel
```

**For Arch Linux:**

```bash 
sudo pacman -S gtest
```

Follow any additional setup steps for your system if required.

## How to Run Tests

The inLimbo project uses CMake to build and manage the test suite. Here’s how to set up and run tests:

1. **Navigate to the `TESTING` Directory:**

   ```bash
   cd tests/audio # go to any one
   ```

2. **Create a Build Directory:**

   It’s a good practice to build in a separate directory to keep the source tree clean.

   ```bash
   cmake -S . -B build-audio-test
   ```

3. **Run CMake:**

   Generate the build system files using CMake:

   ```bash
   cmake --build build-audio-test/
   ```

4. **Build the Tests:**

   Compile the test binaries:

   ```bash
   make
   ```

5. **Run the Tests:**

   Execute the test suite:

   ```bash
   ./MiniAudioPlayerTests
   ```

   Alternatively, if `CTest` is enabled, you can run all tests:

   ```bash
   ctest
   ```

> [!NOTE]
> 
> This is still early in development and I am learning it myself as I go.
> 
> If you feel like these tests are not being used to their full potential, or any form of logical / implementational flaws,
> feel free to open an [issue](https://github.com/nots1dd/inLimbo/issues)
> 
