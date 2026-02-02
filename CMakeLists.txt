cmake_minimum_required(VERSION 3.5...3.20)
project(inLimbo LANGUAGES C CXX)

# -----------------------------------------------------------
# Build configuration
# -----------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -g)

set(INLIMBO_USER_FRONTEND_DIR
    "$ENV{HOME}/.local/share/inLimbo/frontends"
)

# -----------------------------------------------------------
# External Dependencies
# -----------------------------------------------------------
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
set(CLI11_INCLUDE_DIRS ${EXTERNAL_DIR}/CLI11/include)
set(TOMLPLUSPLUS_INCLUDE_DIRS ${EXTERNAL_DIR}/tomlplusplus)
set(CEREAL_INCLUDE_DIRS ${EXTERNAL_DIR}/cereal/include)
set(ANKERL_UNORDERED_DENSE_DIR ${EXTERNAL_DIR}/unordered_dense)
set(ANKERL_UNORDERED_DENSE_INCLUDE_DIRS
    ${ANKERL_UNORDERED_DENSE_DIR}/include
)

# -----------------------------------------------------------
# Local libbacktrace (Debug only)
# -----------------------------------------------------------
set(BACKTRACE_PREFIX "${CMAKE_SOURCE_DIR}/external/libbacktrace/build/install")
set(BACKTRACE_INCLUDE_DIR "${BACKTRACE_PREFIX}/include")
set(BACKTRACE_LIBRARY_DIR "${BACKTRACE_PREFIX}/lib")
set(BACKTRACE_LIBRARY "${BACKTRACE_LIBRARY_DIR}/libbacktrace.a")

set(SPDLOG_GH_URL https://github.com/gabime/spdlog.git)

# --- TOML++ (header-only)
add_library(tomlplusplus INTERFACE)
target_include_directories(tomlplusplus INTERFACE ${TOMLPLUSPLUS_INCLUDE_DIRS})

# --- Cereal (header-only)
add_library(cereal INTERFACE)
target_include_directories(cereal INTERFACE ${CEREAL_INCLUDE_DIRS})

# --- CLI11 (header-only)
add_library(cli11 INTERFACE)
target_include_directories(cli11 INTERFACE ${CLI11_INCLUDE_DIRS})

# --- spdlog (compiled)
set(SPDLOG_DIR ${EXTERNAL_DIR}/spdlog)

# --- ankerl::unordered_dense (header-only)
add_library(ankerl_unordered_dense INTERFACE)
target_include_directories(ankerl_unordered_dense
  INTERFACE ${ANKERL_UNORDERED_DENSE_INCLUDE_DIRS}
)

if(NOT EXISTS ${SPDLOG_DIR}/CMakeLists.txt)
    message(STATUS "spdlog not found — cloning into ${SPDLOG_DIR}")
    execute_process(
        COMMAND git clone --branch master ${SPDLOG_GH_URL} ${SPDLOG_DIR}
        WORKING_DIRECTORY ${EXTERNAL_DIR}
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT EXISTS "${BACKTRACE_LIBRARY}")
        message(FATAL_ERROR
            "libbacktrace not found at:\n"
            "  ${BACKTRACE_LIBRARY}\n"
            "Run:\n"
            "  make init-dep-backtrace"
        )
    endif()
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
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(FFTW3F REQUIRED fftw3f)

# Make MPRIS C bindings
include(${CMAKE_SOURCE_DIR}/cmake/mpris/mpris-c.cmake)

# -----------------------------------------------------------
# Version Information
# -----------------------------------------------------------
include(${CMAKE_SOURCE_DIR}/cmake/Version.cmake)

# -----------------------------------------------------------
# Backend Shared Library
# -----------------------------------------------------------
set(INLIMBO_CORE_SOURCES
    src/core/SongTree.cc
    src/taglib/Parser.cc
    src/mpris/Service.cc
    src/query/SongMap.cc
    src/helpers/cmdline/Display.cc
    src/helpers/fs/Directory.cc
    src/utils/DirectoryWalker.cc
    src/utils/signal/Handler.cc
    src/utils/string/Equals.cc
    src/utils/string/Transforms.cc
    src/config/Watcher.cc
    src/config/Keybinds.cc
    src/config/Colors.cc
    src/config/Misc.cc
    src/telemetry/Sink.cc
    src/telemetry/Analysis.cc
    src/telemetry/Store.cc
    src/telemetry/IDs.cc
    src/toml/Parser.cc
    src/audio/Engine.cc
    src/audio/Service.cc
    src/audio/Playlist.cc
    src/Logger.cc
    src/Context.cc
)

add_library(inLimbo-core SHARED ${INLIMBO_CORE_SOURCES})

target_include_directories(inLimbo-core
    PUBLIC
        include/inlimbo
        ${CMAKE_SOURCE_DIR}
        ${TAGLIB_INCLUDE_DIRS}
        ${ANKERL_UNORDERED_DENSE_INCLUDE_DIRS}
        ${TOMLPLUSPLUS_INCLUDE_DIRS}
        ${CLI11_INCLUDE_DIRS}
        ${CEREAL_INCLUDE_DIRS}
        ${ALSA_INCLUDE_DIRS}
        ${AVFORMAT_INCLUDE_DIRS}
        ${AVCODEC_INCLUDE_DIRS}
        ${AVUTIL_INCLUDE_DIRS}
        ${SWRESAMPLE_INCLUDE_DIRS}
        ${FFTW3F_INCLUDE_DIRS}
        $<$<CONFIG:Debug>:${BACKTRACE_INCLUDE_DIR}>
)

target_link_libraries(inLimbo-core
    PUBLIC
        ankerl_unordered_dense
        inLimbo-mpris-c
        ${TAGLIB_LIBRARIES}
        spdlog::spdlog
        ${AVFORMAT_LIBRARIES}
        ${AVCODEC_LIBRARIES}
        ${AVUTIL_LIBRARIES}
        ${SWRESAMPLE_LIBRARIES}
        ${ALSA_LIBRARIES}
        ${FFTW3F_LIBRARIES}
        $<$<CONFIG:Debug>:${BACKTRACE_LIBRARY}>
)

target_compile_definitions(inLimbo-core
    PRIVATE
        $<$<CONFIG:Debug>:INLIMBO_DEBUG_BUILD>
)

set_target_properties(inLimbo-core PROPERTIES
    OUTPUT_NAME "inLimbo-core"
    POSITION_INDEPENDENT_CODE ON
)

# -----------------------------------------------------------
# Frontend registry (global)
# -----------------------------------------------------------
set(INLIMBO_FRONTEND_NAMES "")

# ===================== FRONTENDS ============================
include(${CMAKE_SOURCE_DIR}/cmake/frontends/cmdline.cmake)
# include(${CMAKE_SOURCE_DIR}/cmake/frontends/raylib.cmake)
# ===================== FRONTENDS ============================

include(${CMAKE_SOURCE_DIR}/cmake/FrontendPluginList.cmake)

# -----------------------------------------------------------
# Test Executable
# -----------------------------------------------------------
set(SOURCES
  src/inLimbo.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include/inlimbo
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        inLimbo-core
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        $<$<CONFIG:Debug>:INLIMBO_DEBUG_BUILD>
)

# Enable testing
include(CTest)
enable_testing()

# Add tests
add_subdirectory(tests)

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
message(STATUS "${SEP}  • CLI11      : ${CLI11_INCLUDE_DIRS}")
message(STATUS "${SEP}  • spdlog     : ${SPDLOG_DIR}")
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── Multimedia / Audio ───────────────────────────────")
message(STATUS "${SEP}  • libavformat  : ${AVFORMAT_VERSION} (${AVFORMAT_LIBRARIES})")
message(STATUS "${SEP}  • libavcodec   : ${AVCODEC_VERSION} (${AVCODEC_LIBRARIES})")
message(STATUS "${SEP}  • libavutil    : ${AVUTIL_VERSION} (${AVUTIL_LIBRARIES})")
message(STATUS "${SEP}  • libswresample: ${SWRESAMPLE_VERSION} (${SWRESAMPLE_LIBRARIES})")
message(STATUS "${SEP}  • ALSA         : ${ALSA_VERSION} (${ALSA_LIBRARIES})")
message(STATUS "${SEP}  • FFTW         : ${FFTW3F_VERSION} (${FFTW3F_LIBRARIES})")
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── System Libraries ────────────────────────────────────")
message(STATUS "${SEP}  • TagLib     : ${TAGLIB_INCLUDE_DIRS}")
message(STATUS "${SEP}  • backtrace  : ${BACKTRACE_LIBRARY} (Debug builds only)")
message(STATUS "${SEP}  • DBUS-1     : ${DBUS_INCLUDE_DIRS}")
message(STATUS "${SEP}")
message(STATUS "${LINE_BOTTOM}\n")
