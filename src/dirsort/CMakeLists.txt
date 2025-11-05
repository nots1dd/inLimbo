cmake_minimum_required(VERSION 3.20)
project(dirsort-test LANGUAGES CXX)

# -----------------------------------------------------------
# Build configuration
# -----------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -g)

# -----------------------------------------------------------
# Source files
# -----------------------------------------------------------
set(SOURCES
    test.cpp
)

# -----------------------------------------------------------
# External Dependencies
# -----------------------------------------------------------
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/../../external)

# --- TOML++ (header-only)
add_library(tomlplusplus INTERFACE)
target_include_directories(tomlplusplus INTERFACE ${EXTERNAL_DIR}/tomlplusplus)

# --- Miniaudio (single header)
add_library(miniaudio INTERFACE)
target_include_directories(miniaudio INTERFACE ${EXTERNAL_DIR}/miniaudio)

# --- Cereal (header-only)
add_library(cereal INTERFACE)
target_include_directories(cereal INTERFACE ${EXTERNAL_DIR}/cereal/include/cereal)

# --- spdlog (compiled)
set(SPDLOG_DIR ${EXTERNAL_DIR}/spdlog)

# Clone spdlog automatically if not found
if(NOT EXISTS ${SPDLOG_DIR}/CMakeLists.txt)
    message(STATUS "spdlog not found — cloning into ${SPDLOG_DIR}")
    execute_process(
        COMMAND git clone --branch master https://github.com/gabime/spdlog.git ${SPDLOG_DIR}
        WORKING_DIRECTORY ${EXTERNAL_DIR}
    )
endif()

# Build spdlog as a proper target
add_subdirectory(${SPDLOG_DIR} ${CMAKE_BINARY_DIR}/spdlog_build EXCLUDE_FROM_ALL)

# -----------------------------------------------------------
# System Dependencies via pkg-config
# -----------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)

# -----------------------------------------------------------
# Target setup
# -----------------------------------------------------------
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ../../include/inlimbo
    ${TAGLIB_INCLUDE_DIRS}
    ${EXTERNAL_DIR}/tomlplusplus
)

target_link_libraries(${PROJECT_NAME}
    ${TAGLIB_LIBRARIES}
    spdlog::spdlog
    backtrace
)

# -----------------------------------------------------------
# RPATH & Build Options (Linux)
# -----------------------------------------------------------
if(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH_USE_ORIGIN ON
        SKIP_BUILD_RPATH FALSE
    )
endif()

# -----------------------------------------------------------
# Diagnostics Summary
# -----------------------------------------------------------
set(LINE_TOP    "┌───────────────────────────── Build Summary ─────────────────────────────┐")
set(LINE_BOTTOM "└──────────────────────────────────────────────────────────────────────────┘")
set(SEP         "│")

message(STATUS "${LINE_TOP}")
message(STATUS "${SEP}  Project Name     : ${PROJECT_NAME}")
message(STATUS "${SEP}  Build Type       : ${CMAKE_BUILD_TYPE}")
message(STATUS "${SEP}  System           : ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "${SEP}  Compiler         : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "${SEP}  C++ Standard     : C++${CMAKE_CXX_STANDARD}")
message(STATUS "${SEP}  Build Directory  : ${CMAKE_BINARY_DIR}")
message(STATUS "${SEP}  Source Directory : ${CMAKE_SOURCE_DIR}")
message(STATUS "${SEP}  Executable Path  : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── External Dependencies ───────────────────────────────")
message(STATUS "${SEP}  • TOML++     : ${EXTERNAL_DIR}/tomlplusplus")
message(STATUS "${SEP}  • miniaudio  : ${EXTERNAL_DIR}/miniaudio")
message(STATUS "${SEP}  • cereal     : ${EXTERNAL_DIR}/cereal/include/cereal")
message(STATUS "${SEP}  • spdlog     : ${SPDLOG_DIR}")
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── System Libraries ────────────────────────────────────")
message(STATUS "${SEP}  • TagLib     : ${TAGLIB_INCLUDE_DIRS}")
message(STATUS "${SEP}  • backtrace  : linked manually")
message(STATUS "${LINE_BOTTOM}\n")
