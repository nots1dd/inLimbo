# ===========================================================
# Raylib frontend
# ===========================================================

set(INLIMBO_FRONTEND_TARGET      inLimbo-fe-raylib)
set(INLIMBO_FRONTEND_NAME        "raylib")
set(INLIMBO_FRONTEND_OUTPUT_NAME "inlimbo-frontend-raylib")

find_package(raylib REQUIRED)

set(INLIMBO_FE_RAYLIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/Interface.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/input/Handler.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/media/AlbumArtCache.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/ui/Fonts.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/ui/TextUtils.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/util/Scroll.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/view/Header.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/view/NowPlaying.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/view/AlbumsView.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/view/ArtistsView.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/view/MetadataOverlay.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/view/StatusBar.cc
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/PluginShim.cc
)

add_library(${INLIMBO_FRONTEND_TARGET} SHARED
    ${INLIMBO_FE_RAYLIB_SOURCES}
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

target_compile_definitions(${INLIMBO_FRONTEND_TARGET}
    PRIVATE
        INLIMBO_FRONTEND_RAYLIB
)

target_link_libraries(${INLIMBO_FRONTEND_TARGET}
    PUBLIC
        inLimbo-core
        raylib
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
