cmake_minimum_required(VERSION 3.20)
project(inLimbo LANGUAGES CXX)

# -----------------------------------------------------------
# Build configuration
# -----------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -g)

# -----------------------------------------------------------
# External Dependencies
# -----------------------------------------------------------
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
set(CXXOPTS_INCLUDE_DIRS ${EXTERNAL_DIR}/cxxopts/include)
set(TOMLPLUSPLUS_INCLUDE_DIRS ${EXTERNAL_DIR}/tomlplusplus)
set(CEREAL_INCLUDE_DIRS ${EXTERNAL_DIR}/cereal/include)

set(SPDLOG_GH_URL https://github.com/gabime/spdlog.git)

# --- TOML++ (header-only)
add_library(tomlplusplus INTERFACE)
target_include_directories(tomlplusplus INTERFACE ${TOMLPLUSPLUS_INCLUDE_DIRS})

# --- Cereal (header-only)
add_library(cereal INTERFACE)
target_include_directories(cereal INTERFACE ${CEREAL_INCLUDE_DIRS})

# --- cxxopts (header-only)
add_library(cxxopts INTERFACE)
target_include_directories(cxxopts INTERFACE ${CXXOPTS_INCLUDE_DIRS})

# --- spdlog (compiled)
set(SPDLOG_DIR ${EXTERNAL_DIR}/spdlog)

if(NOT EXISTS ${SPDLOG_DIR}/CMakeLists.txt)
    message(STATUS "spdlog not found — cloning into ${SPDLOG_DIR}")
    execute_process(
        COMMAND git clone --branch master ${SPDLOG_GH_URL} ${SPDLOG_DIR}
        WORKING_DIRECTORY ${EXTERNAL_DIR}
    )
endif()

add_subdirectory(${SPDLOG_DIR} ${CMAKE_BINARY_DIR}/spdlog_build EXCLUDE_FROM_ALL)
set_target_properties(spdlog PROPERTIES POSITION_INDEPENDENT_CODE ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -----------------------------------------------------------
# System Dependencies
# -----------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC  REQUIRED libavcodec)
pkg_check_modules(AVUTIL   REQUIRED libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
pkg_check_modules(ALSA REQUIRED alsa)

# -----------------------------------------------------------
# Backend Shared Library
# -----------------------------------------------------------
set(INLIMBO_CORE_SOURCES
    src/utils/RBTree.cc
    src/core/SongTree.cc
    src/core/taglib/Parser.cc
    src/core/DirectoryWalker.cc
    src/audio/Playback.cc
)

add_library(inLimbo-core SHARED ${INLIMBO_CORE_SOURCES})

target_include_directories(inLimbo-core
    PUBLIC
        include/inlimbo
        ${TAGLIB_INCLUDE_DIRS}
        ${TOMLPLUSPLUS_INCLUDE_DIRS}
        ${CXXOPTS_INCLUDE_DIRS}
        ${CEREAL_INCLUDE_DIRS}
)

target_link_libraries(inLimbo-core
    PUBLIC
        ${TAGLIB_LIBRARIES}
        spdlog::spdlog
        backtrace
        avformat
        swresample
        avutil
        avcodec
        asound
)

set_target_properties(inLimbo-core PROPERTIES
    OUTPUT_NAME "inLimbo-core"
    POSITION_INDEPENDENT_CODE ON
)

# -----------------------------------------------------------
# Test Executable
# -----------------------------------------------------------
set(SOURCES     
  src/test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include/inlimbo
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        inLimbo-core
)

# -----------------------------------------------------------
# RPATH & Build Options
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
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── External Dependencies ───────────────────────────────")
message(STATUS "${SEP}  • TOML++     : ${TOMLPLUSPLUS_INCLUDE_DIRS}")
message(STATUS "${SEP}  • cereal     : ${CEREAL_INCLUDE_DIRS}")
message(STATUS "${SEP}  • cxxopts    : ${CXXOPTS_INCLUDE_DIRS}")
message(STATUS "${SEP}  • spdlog     : ${SPDLOG_DIR}")
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── Multimedia / Audio ───────────────────────────────")
message(STATUS "${SEP}  • libavformat  : ${AVFORMAT_VERSION} (${AVFORMAT_LIBRARIES})")
message(STATUS "${SEP}  • libavcodec   : ${AVCODEC_VERSION} (${AVCODEC_LIBRARIES})")
message(STATUS "${SEP}  • libavutil    : ${AVUTIL_VERSION} (${AVUTIL_LIBRARIES})")
message(STATUS "${SEP}  • libswresample: ${SWRESAMPLE_VERSION} (${SWRESAMPLE_LIBRARIES})")
message(STATUS "${SEP}  • ALSA         : ${ALSA_VERSION} (${ALSA_LIBRARIES})")
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── System Libraries ────────────────────────────────────")
message(STATUS "${SEP}  • TagLib     : ${TAGLIB_INCLUDE_DIRS}")
message(STATUS "${SEP}  • backtrace  : linked manually")
message(STATUS "${LINE_BOTTOM}\n")
