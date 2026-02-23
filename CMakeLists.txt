cmake_minimum_required(VERSION 3.5...4.2.1)
project(inLimbo LANGUAGES C CXX)

# -----------------------------------------------------------
# ccache detect
# -----------------------------------------------------------

find_program(INLIMBO_CCACHE_PROGRAM ccache)

if(INLIMBO_CCACHE_PROGRAM)

    message(STATUS "ccache found: ${INLIMBO_CCACHE_PROGRAM}")
    message(STATUS "ccache enabled for C/C++ compilation")

    set(CMAKE_C_COMPILER_LAUNCHER   ${INLIMBO_CCACHE_PROGRAM})
    set(CMAKE_CXX_COMPILER_LAUNCHER ${INLIMBO_CCACHE_PROGRAM})

else()

    message(STATUS "ccache not found — builds will proceed without cache")

endif()

# -----------------------------------------------------------
# Build configuration
# -----------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------
# Sanitizers (Debug only)
# -----------------------------------------------------------
option(INLIMBO_ENABLE_ASAN "Enable AddressSanitizer in Debug builds" ON)
option(INLIMBO_ENABLE_TEST "Build unit tests" ON)

# -----------------------------------------------------------
# Build-Type Specific Compiler Options
# -----------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Release")

    message(STATUS "Configuring RELEASE compiler flags")

    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )

    add_compile_options(
        -O2
        -DNDEBUG
    )

    add_compile_options(-march=native)

elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")

    message(STATUS "Configuring RELWITHDEBINFO compiler flags")

    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -O2
        -g
        -DNDEBUG
    )

elseif(INLIMBO_ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")

    message(STATUS "AddressSanitizer ENABLED")

    set(SANITIZER_FLAGS
        -fsanitize=address
        -fsanitize=undefined
        -fno-omit-frame-pointer
        -fno-optimize-sibling-calls
    )

    add_compile_options(${SANITIZER_FLAGS})
    add_link_options(${SANITIZER_FLAGS})

endif()

if(INLIMBO_ENABLE_ASAN AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(
        ASAN_ENABLED=1
    )
endif()

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
set(NLOHMANN_JSON_INCLUDE_DIRS ${EXTERNAL_DIR}/nlohmann_json/include)
set(ANKERL_UNORDERED_DENSE_DIR ${EXTERNAL_DIR}/unordered_dense)
set(ANKERL_UNORDERED_DENSE_INCLUDE_DIRS
    ${ANKERL_UNORDERED_DENSE_DIR}/include
)

# -----------------------------------------------------------
# Local libbacktrace (Debug only)
# -----------------------------------------------------------
set(BACKTRACE_PREFIX "${CMAKE_SOURCE_DIR}/external/libbacktrace/build/install")
set(BACKTRACE_INCLUDE_DIRS "${BACKTRACE_PREFIX}/include")
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

# --- nlohmann::json (header-only)
add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE ${NLOHMANN_JSON_INCLUDE_DIRS})

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
        COMMAND git clone --progress --branch master ${SPDLOG_GH_URL} ${SPDLOG_DIR}
        WORKING_DIRECTORY ${EXTERNAL_DIR}
        RESULT_VARIABLE GIT_RESULT
    )

    if(NOT GIT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to clone spdlog")
    endif()
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
find_package(OpenSSL REQUIRED)
pkg_check_modules(TAGLIB REQUIRED taglib)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC  REQUIRED libavcodec)
pkg_check_modules(AVUTIL   REQUIRED libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(FFTW3F REQUIRED fftw3f)

message(STATUS "OpenSSL version : ${OpenSSL_VERSION}")
message(STATUS "OpenSSL include : ${OPENSSL_INCLUDE_DIR}")

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
    src/core/SongLibrarySnapshot.cc
    src/audio/backend/alsa/Impl.cc
    src/audio/backend/Interface.cc
    src/audio/Registry.cc
    src/audio/Service.cc
    src/audio/Playlist.cc
    src/taglib/Parser.cc
    src/taglib/Utils.cc
    src/taglib/source/MP3.cc
    src/taglib/source/FLAC.cc
    src/mpris/Service.cc
    src/telemetry/Sink.cc
    src/telemetry/Analysis.cc
    src/telemetry/Store.cc
    src/telemetry/IDs.cc
    src/query/SongMap.cc
    src/query/sort/Engine.cc
    src/query/sort/Stats.cc
    src/frontend/Plugin.cc
    src/helpers/cmdline/Display.cc
    src/helpers/fs/Directory.cc
    src/helpers/telemetry/Playback.cc
    src/utils/DirectoryWalker.cc
    src/utils/signal/Handler.cc
    src/utils/string/Equals.cc
    src/utils/string/Transforms.cc
    src/utils/unix/net/HTTPSClient.cc
    src/config/Config.cc
    src/config/sort/Model.cc
    src/config/sort/ParseMetrics.cc
    src/config/Watcher.cc
    src/config/Keybinds.cc
    src/config/Colors.cc
    src/config/Misc.cc
    src/lrc/Client.cc
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
        ${NLOHMANN_JSON_INCLUDE_DIRS}
        ${CEREAL_INCLUDE_DIRS}
        ${ALSA_INCLUDE_DIRS}
        ${AVFORMAT_INCLUDE_DIRS}
        ${AVCODEC_INCLUDE_DIRS}
        ${AVUTIL_INCLUDE_DIRS}
        ${SWRESAMPLE_INCLUDE_DIRS}
        ${FFTW3F_INCLUDE_DIRS}
        $<$<CONFIG:Debug>:${BACKTRACE_INCLUDE_DIRS}>
)

target_link_libraries(inLimbo-core
    PUBLIC
        ankerl_unordered_dense
        inLimbo-mpris-c
        ${TAGLIB_LIBRARIES}
        spdlog::spdlog
        OpenSSL::SSL
        OpenSSL::Crypto
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
include(${CMAKE_SOURCE_DIR}/cmake/frontends/ftxui.cmake)
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

if(CMAKE_BUILD_TYPE STREQUAL "Release")

    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)

    if(IPO_SUPPORTED)

        message(STATUS ">> Enabling LTO for core targets")

        set_property(TARGET inLimbo-core PROPERTY
            INTERPROCEDURAL_OPTIMIZATION TRUE)

        set_property(TARGET ${PROJECT_NAME} PROPERTY
            INTERPROCEDURAL_OPTIMIZATION TRUE)

    else()
        message(WARNING "LTO not supported: ${IPO_ERROR}")
    endif()

endif()

if(INLIMBO_ENABLE_TEST)

    message(STATUS "Tests ENABLED")

    include(CTest)
    enable_testing()

    add_subdirectory(tests)

else()

    message(STATUS "Tests DISABLED")

endif()

# -----------------------------------------------------------
# RPATH & Build Options
# -----------------------------------------------------------
if(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH_USE_ORIGIN ON
        SKIP_BUILD_RPATH FALSE
    )
endif()

get_target_property(CORE_COMPILE_OPTS inLimbo-core COMPILE_OPTIONS)

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
message(STATUS "${SEP}  ── Compiler Flags (inLimbo/inLimbo-core) ───────────────────────────────")

message(STATUS "${SEP}  • C Flags           : ${CMAKE_C_FLAGS}")
message(STATUS "${SEP}  • C Flags Debug     : ${CMAKE_C_FLAGS_DEBUG}")
message(STATUS "${SEP}  • C Flags Release   : ${CMAKE_C_FLAGS_RELEASE}")
message(STATUS "${SEP}  • CXX Flags         : ${CMAKE_CXX_FLAGS}")
message(STATUS "${SEP}  • CXX Flags Debug   : ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "${SEP}  • CXX Flags Release : ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "${SEP}  • All flags         :")
foreach(opt ${CORE_COMPILE_OPTS})
    message(STATUS "${SEP}                      > ${opt}")
endforeach()
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── External Dependencies ───────────────────────────────")
message(STATUS "${SEP}  • TOML++        : ${TOMLPLUSPLUS_INCLUDE_DIRS}")
message(STATUS "${SEP}  • cereal        : ${CEREAL_INCLUDE_DIRS}")
message(STATUS "${SEP}  • CLI11         : ${CLI11_INCLUDE_DIRS}")
message(STATUS "${SEP}  • libbacktrace  : '${BACKTRACE_INCLUDE_DIRS}' (Debug builds only)")
message(STATUS "${SEP}  • spdlog        : ${SPDLOG_DIR}")
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── Multimedia / Audio (System Libraries) ───────────────────────────────")
message(STATUS "${SEP}  • libavformat   : ${AVFORMAT_VERSION} (${AVFORMAT_LIBRARIES})")
message(STATUS "${SEP}  • libavcodec    : ${AVCODEC_VERSION} (${AVCODEC_LIBRARIES})")
message(STATUS "${SEP}  • libavutil     : ${AVUTIL_VERSION} (${AVUTIL_LIBRARIES})")
message(STATUS "${SEP}  • libswresample : ${SWRESAMPLE_VERSION} (${SWRESAMPLE_LIBRARIES})")
message(STATUS "${SEP}  • ALSA          : ${ALSA_VERSION} (${ALSA_LIBRARIES})")
message(STATUS "${SEP}  • FFTW          : ${FFTW3F_VERSION} (${FFTW3F_LIBRARIES})")
message(STATUS "${SEP}")
message(STATUS "${SEP}  ── Misc (System Libraries) ────────────────────────────────────")
message(STATUS "${SEP}  • TagLib        : ${TAGLIB_INCLUDE_DIRS}")
message(STATUS "${SEP}  • DBUS-1        : ${DBUS_INCLUDE_DIRS}")
message(STATUS "${SEP}")
message(STATUS "${LINE_BOTTOM}\n")
