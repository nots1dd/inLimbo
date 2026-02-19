# ===========================================================
# FTXUI frontend
# ===========================================================

set(INLIMBO_FRONTEND_TARGET      inLimbo-fe-ftxui)
set(INLIMBO_FRONTEND_NAME        "ftxui")
set(INLIMBO_FRONTEND_OUTPUT_NAME "inlimbo-frontend-ftxui")

set(INLIMBO_FE_FTXUI_SOURCES
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/Interface.cc
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/PluginShim.cc

  # managers
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/managers/Threads.cc

  # state
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/state/album_art/Impl.cc
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/state/library/Impl.cc
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/state/now_playing/Impl.cc
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/state/queue/Impl.cc

  # ui::screens
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/ui/screens/Main.cc
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/ui/screens/NowPlaying.cc
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/ui/screens/Queue.cc
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/ui/EventHandler.cc

  # components
  ${CMAKE_SOURCE_DIR}/src/frontend/ftxui/components/scroll/Scrollable.cc
)

include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG        v6.0.0
  GIT_PROGRESS   TRUE
  GIT_SHALLOW    TRUE
  EXCLUDE_FROM_ALL
)

set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(FTXUI_BUILD_TESTS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(ftxui)

# ===========================================================
# stb_image (header only)
# ===========================================================

FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(stb)

add_library(stb_image INTERFACE)

target_include_directories(stb_image
    INTERFACE
        ${stb_SOURCE_DIR}
)

# -------------------- NOTE ----------------------
# NOW THE REST OF THE FILE CAN BE IGNORED... (except for line 59)
# -------------------- NOTE ----------------------

add_library(${INLIMBO_FRONTEND_TARGET} SHARED
  ${INLIMBO_FE_FTXUI_SOURCES}
)

set_target_properties(${INLIMBO_FRONTEND_TARGET} PROPERTIES
  OUTPUT_NAME ${INLIMBO_FRONTEND_OUTPUT_NAME}
    PREFIX "lib"
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(${INLIMBO_FRONTEND_TARGET}
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/inlimbo
        ${CMAKE_SOURCE_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_include_directories(${INLIMBO_FRONTEND_TARGET}
        PUBLIC ${BACKTRACE_INCLUDE_DIR}
    )
endif()

target_compile_definitions(${INLIMBO_FRONTEND_TARGET}
    PRIVATE
    INLIMBO_FRONTEND_FTXUI
        $<$<CONFIG:Debug>:INLIMBO_DEBUG_BUILD>
)

target_link_libraries(${INLIMBO_FRONTEND_TARGET}
    PRIVATE
        inLimbo-core
        ftxui::screen
        ftxui::dom
        ftxui::component
        stb_image
)

include(GNUInstallDirs)

add_custom_command(
    TARGET ${INLIMBO_FRONTEND_TARGET}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "${INLIMBO_USER_FRONTEND_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${INLIMBO_FRONTEND_TARGET}>
            "${INLIMBO_USER_FRONTEND_DIR}/$<TARGET_FILE_NAME:${INLIMBO_FRONTEND_TARGET}>"
            COMMENT ">> Deploying '${INLIMBO_FRONTEND_NAME}' frontend plugin to ${INLIMBO_USER_FRONTEND_DIR}"
)

list(APPEND INLIMBO_FRONTEND_NAMES "${INLIMBO_FRONTEND_NAME}")
