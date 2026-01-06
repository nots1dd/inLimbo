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

# -----------------------------------------------------------
# External Dependencies
# -----------------------------------------------------------
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
set(CXXOPTS_INCLUDE_DIRS ${EXTERNAL_DIR}/cxxopts/include)
set(TOMLPLUSPLUS_INCLUDE_DIRS ${EXTERNAL_DIR}/tomlplusplus)
set(CEREAL_INCLUDE_DIRS ${EXTERNAL_DIR}/cereal/include)

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

# -----------------------------------------------------------
# Pure C MPRIS (libdbus)
# -----------------------------------------------------------
set(INLIMBO_MPRIS_C_SOURCES
    src/c/mpris.c
)

add_library(inLimbo-mpris-c STATIC ${INLIMBO_MPRIS_C_SOURCES})

set_target_properties(inLimbo-mpris-c PROPERTIES
    LINKER_LANGUAGE C
)

target_include_directories(inLimbo-mpris-c
    PUBLIC
        include
        include/inlimbo
        ${DBUS_INCLUDE_DIRS}
)

target_compile_options(inLimbo-mpris-c
    PRIVATE
        ${DBUS_CFLAGS_OTHER}
)

target_link_libraries(inLimbo-mpris-c
    PUBLIC
        ${DBUS_LIBRARIES}
)

# -----------------------------------------------------------
# Backend Shared Library
# -----------------------------------------------------------
set(INLIMBO_CORE_SOURCES
    src/core/SongTree.cc
    src/core/taglib/Parser.cc
    src/mpris/Service.cc
    src/query/SongMap.cc
    src/helpers/cmdline/Display.cc
    src/helpers/fs/Directory.cc
    src/utils/DirectoryWalker.cc
    src/utils/signal/Handler.cc
    src/utils/string/Equals.cc
    src/toml/Parser.cc
    src/audio/Engine.cc
    src/audio/Service.cc
    src/CmdLine.cc
    src/Logger.cc
)

add_library(inLimbo-core SHARED ${INLIMBO_CORE_SOURCES})

target_include_directories(inLimbo-core
    PUBLIC
        include/inlimbo
        ${CMAKE_SOURCE_DIR}
        ${TAGLIB_INCLUDE_DIRS}
        ${TOMLPLUSPLUS_INCLUDE_DIRS}
        ${CXXOPTS_INCLUDE_DIRS}
        ${CEREAL_INCLUDE_DIRS}
        ${ALSA_INCLUDE_DIRS}
        ${AVFORMAT_INCLUDE_DIRS}
        ${AVCODEC_INCLUDE_DIRS}
        ${AVUTIL_INCLUDE_DIRS}
        ${SWRESAMPLE_INCLUDE_DIRS}
        $<$<CONFIG:Debug>:${BACKTRACE_INCLUDE_DIR}>
)

target_link_libraries(inLimbo-core
    PUBLIC
        inLimbo-mpris-c
        ${TAGLIB_LIBRARIES}
        spdlog::spdlog
        avformat
        swresample
        avutil
        avcodec
        asound
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
# Frontend selection (makefile defaults it to cmdline!)
# -----------------------------------------------------------
if(INLIMBO_FRONTEND_TYPE STREQUAL "cmdline")

    set(INLIMBO_FE_CMDLINE_SOURCES
        src/frontend/cmdline/Interface.cc
    )

    add_library(inLimbo-fe-cmdline SHARED ${INLIMBO_FE_CMDLINE_SOURCES})

    target_include_directories(inLimbo-fe-cmdline
        PUBLIC
            include/inlimbo
            ${CMAKE_SOURCE_DIR}
            $<$<CONFIG:Debug>:${BACKTRACE_INCLUDE_DIR}>
    )

    target_compile_definitions(inLimbo-fe-cmdline
        PRIVATE
            $<$<CONFIG:Debug>:INLIMBO_DEBUG_BUILD>
            INLIMBO_FRONTEND_CMDLINE
    )

    target_link_libraries(inLimbo-fe-cmdline
        PUBLIC
            inLimbo-core
    )

    set(INLIMBO_FRONTEND_TARGET inLimbo-fe-cmdline)
    set(INLIMBO_FRONTEND_NAME   "cmdline")

endif()

# -----------------------------------------------------------
# Test Executable
# -----------------------------------------------------------
set(SOURCES     
  src/Context.cc
  src/test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        include/inlimbo
        ${CMAKE_SOURCE_DIR}
)

if(INLIMBO_FRONTEND_TYPE STREQUAL "none")
  message(FATAL_ERROR "No frontend will be built.")
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        inLimbo-core
        ${INLIMBO_FRONTEND_TARGET}
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
message(STATUS "${SEP}  • backtrace  : ${BACKTRACE_LIBRARY} (Debug builds only)")
message(STATUS "${SEP}  • DBUS-1     : ${DBUS_INCLUDE_DIRS}")
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── Frontend ───────────────────────────────────────────")
message(STATUS "${SEP}  • Type        : ${INLIMBO_FRONTEND_NAME}")
message(STATUS "${SEP}  • Target      : ${INLIMBO_FRONTEND_TARGET}")
message(STATUS "${LINE_BOTTOM}\n")
