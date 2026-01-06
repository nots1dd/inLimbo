# ===========================================================
# Pure C MPRIS (libdbus)
# ===========================================================

set(INLIMBO_MPRIS_C_SOURCES
    src/c/mpris.c
)

add_library(inLimbo-mpris-c STATIC
    ${INLIMBO_MPRIS_C_SOURCES}
)

set_target_properties(inLimbo-mpris-c PROPERTIES
    LINKER_LANGUAGE C
)

target_include_directories(inLimbo-mpris-c
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/include/inlimbo
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
