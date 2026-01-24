# tests/CMakeLists.txt

# Fetch GoogleTest (recommended way)
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# Prevent overriding parent project compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# Make GTest available to subdirs
set(GTEST_LIBS
  gtest
  gtest_main
  gmock
  gmock_main
)

# Add each test subject
add_subdirectory(smallstring)
add_subdirectory(bench)
