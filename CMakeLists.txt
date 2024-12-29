cmake_minimum_required (VERSION 3.22)

project(inLimbo
  LANGUAGES CXX
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

# --- Add Executable --------------------------------------------------------
add_executable(inLimbo
  src/main.cpp
  dom/color_info_sorted_2d.ipp
)

# Set include directories for source files
target_include_directories(inLimbo PRIVATE src)

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
  # Native build: Link to additional libraries (e.g., TagLib)
  find_package(TagLib REQUIRED)
  target_link_libraries(inLimbo PRIVATE TagLib::TagLib)
endif()

# Link libraries for native build
target_link_libraries(inLimbo
  PRIVATE ftxui::screen
  PRIVATE ftxui::dom
  PRIVATE ftxui::component
)

# --- Ensure the config.toml file is created in the correct directory ---------------------
if (NOT EXISTS "$ENV{HOME}/.config/inLimbo/config.toml")
  file(MAKE_DIRECTORY "$ENV{HOME}/.config/inLimbo")
  configure_file("${CMAKE_SOURCE_DIR}/src/parser/examples/config.toml" "$ENV{HOME}/.config/inLimbo/config.toml" COPYONLY)
endif()
