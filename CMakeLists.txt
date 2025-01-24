cmake_minimum_required (VERSION 3.22)

project(inLimbo
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_library(tiv
  ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/components/libs/tiv_lib.cpp
)

# Option to select between local or global build
option(BUILD_GLOBAL "Build globally and install" OFF)

# --- Fetch FTXUI --------------------------------------------------------------
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v5.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)
# ------------------------------------------------------------------------------

# --- Set Executable Name -------------------------------------------------------
# Change executable name based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug-ASan")
  set(EXECUTABLE_NAME "inLimbo-DBG-Asan")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug-TSan")
  set(EXECUTABLE_NAME "inLimbo-DBG-TSan")
else()
  set(EXECUTABLE_NAME "inLimbo")
endif()

# --- Add Executable --------------------------------------------------------
add_executable(${EXECUTABLE_NAME}
  src/main.cpp
  src/ui/components/scroller.cpp
  src/ui/components/image_view.cpp
  src/ui/components/libs/tiv_lib.cpp
)

# Set include directories for source files
target_include_directories(${EXECUTABLE_NAME} PRIVATE src)

# --- Debugging Flags --------------------------------------------------------
# Add debugging flags for better error reporting, debugging, and sanitization

if(CMAKE_BUILD_TYPE STREQUAL "Debug-ASan")
  # Enable AddressSanitizer
  message(STATUS "Enabling AddressSanitizer for Debug-AddressSanitizer build")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")

  # Enable UndefinedBehaviorSanitizer (UBSan)
  message(STATUS "Enabling UndefinedBehaviorSanitizer for Debug-AddressSanitizer build")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")

  # Enable Debug Symbols
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

  # Enable all warnings and treat them as errors
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
 
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug-TSan")
  # Enable ThreadSanitizer
  message(STATUS "Enabling ThreadSanitizer for Debug-ThreadSanitizer build")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -g")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=thread")

  # Enable UndefinedBehaviorSanitizer (UBSan)
  message(STATUS "Enabling UndefinedBehaviorSanitizer for Debug-ThreadSanitizer build")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")

  # Enable Debug Symbols
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

  # Enable all warnings and treat them as errors
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

else()
  message(STATUS "Skipping Sanitizers for Release build")
endif()

# --- Handle Emscripten-specific flags and files ---------------------------------
if (EMSCRIPTEN)
  # Emscripten-specific flags
  string(APPEND CMAKE_CXX_FLAGS " -s USE_PTHREADS")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s ASYNCIFY")
  string(APPEND CMAKE_EXE_LINKER_FLAGS " -s PROXY_TO_PTHREAD")

  # Ensure the necessary files are copied for web build
  foreach(file "index.html" "run_webassembly.py")
    configure_file("src/${file}" ${file})
  endforeach(file)

  # Disable TagLib for Emscripten build
  message(STATUS "Building for Emscripten: TagLib will be excluded")
else()
  target_link_libraries(${EXECUTABLE_NAME})
endif()

# --- Handle Global Build ---------------------------------------------------------
if(DEFINED GLOBAL_BUILD AND GLOBAL_BUILD)
  set(CMAKE_INSTALL_PREFIX "/usr/")
  message(STATUS "Starting GLOBAL_BUILD for inLimbo...")

  install(TARGETS ${EXECUTABLE_NAME} DESTINATION bin)

  install(FILES assets/inLimbo.desktop DESTINATION share/applications)
  install(FILES assets/logo/inLimbo.png DESTINATION share/icons/hicolor/256x256/apps)

else()
  # Local build (build in ./build directory)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/build")

  message(STATUS "Building locally, binaries will be located in ./build/")
endif()

# Ensure that pkg-config is available
find_package(PkgConfig REQUIRED)

# Try to find GLib and Gio using pkg-config
pkg_check_modules(GLIB REQUIRED glib-2.0 gio-2.0)

# If pkg-config is not available or fails to find the libraries, fall back to manual search
if (NOT GLIB_FOUND)
    message(STATUS "pkg-config not found or failed to find GLib and Gio, attempting manual search...")

    # Try searching for the libraries using standard system directories
    find_library(GLIB_LIBRARY NAMES glib-2.0 gio-2.0)
    
    if (NOT GLIB_LIBRARY)
        message(FATAL_ERROR "GLib and Gio libraries not found. Install libglib-2.0-dev and libgio-2.0-dev.")
    endif()

    # Manually find the include directories for GLib and Gio
    find_path(GLIB_INCLUDE_DIR glib.h)
    find_path(GIO_INCLUDE_DIR gio.h)

    # If the includes or libraries are not found, give an error
    if (NOT GLIB_INCLUDE_DIR OR NOT GIO_INCLUDE_DIR)
        message(FATAL_ERROR "GLib and Gio include files not found.")
    endif()
    
    set(GLIB_LIBRARIES ${GLIB_LIBRARY})
    set(GLIB_INCLUDE_DIRS ${GLIB_INCLUDE_DIR} ${GIO_INCLUDE_DIR})
endif()

# Add include directories for glib and gio
include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

# Add the necessary compilation and linker flags
add_definitions(${GLIB_CFLAGS_OTHER})

# Link libraries for native build
target_link_libraries(${EXECUTABLE_NAME}
  PRIVATE tiv 
  PRIVATE X11
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
  PRIVATE ${GLIB_LIBRARIES}
  PRIVATE tag
)

# --- Ensure the config.toml file is created in the correct directory ---------------------
if (NOT EXISTS "$ENV{HOME}/.config/inLimbo/config.toml")
  file(MAKE_DIRECTORY "$ENV{HOME}/.config/inLimbo")
  configure_file("${CMAKE_SOURCE_DIR}/src/parser/examples/config.toml" "$ENV{HOME}/.config/inLimbo/config.toml" COPYONLY)
endif()
if (NOT EXISTS "$ENV{HOME}/.cache/inLimbo/")
  file(MAKE_DIRECTORY "$ENV{HOME}/.cache/inLimbo")
endif()

# --- Add Tests Directory Only if INLIMBO_TESTING is Defined ----------------------
if(DEFINED INLIMBO_TESTING AND INLIMBO_TESTING)
  message("--> Enabling TESTING for INLIMBO...")
  add_subdirectory(tests)
else()
  message("--> TESTING is disabled for INLIMBO.")
endif()
