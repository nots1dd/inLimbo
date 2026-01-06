# ===========================================================
# Raylib frontend
# ===========================================================

find_package(raylib REQUIRED)

set(INLIMBO_FE_RAYLIB_SOURCES
    ${CMAKE_SOURCE_DIR}/src/frontend/raylib/Interface.cc
)

add_library(inLimbo-fe-raylib SHARED
    ${INLIMBO_FE_RAYLIB_SOURCES}
)

target_include_directories(inLimbo-fe-raylib
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/inlimbo
        ${CMAKE_SOURCE_DIR}
)

target_compile_definitions(inLimbo-fe-raylib
    PRIVATE
        INLIMBO_FRONTEND_RAYLIB
)

target_link_libraries(inLimbo-fe-raylib
    PUBLIC
        inLimbo-core
        raylib
)

set(INLIMBO_FRONTEND_TARGET inLimbo-fe-raylib)
set(INLIMBO_FRONTEND_NAME "raylib")
