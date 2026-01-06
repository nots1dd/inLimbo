# ===========================================================
# Cmdline frontend
# ===========================================================

set(INLIMBO_FE_CMDLINE_SOURCES
    src/frontend/cmdline/Interface.cc
)

add_library(inLimbo-fe-cmdline STATIC
    ${INLIMBO_FE_CMDLINE_SOURCES}
)

target_include_directories(inLimbo-fe-cmdline
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/inlimbo
        ${CMAKE_SOURCE_DIR}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_include_directories(inLimbo-fe-cmdline
        PUBLIC ${BACKTRACE_INCLUDE_DIR}
    )
endif()

target_compile_definitions(inLimbo-fe-cmdline
    PRIVATE
        INLIMBO_FRONTEND_CMDLINE
        $<$<CONFIG:Debug>:INLIMBO_DEBUG_BUILD>
)

target_link_libraries(inLimbo-fe-cmdline
    PRIVATE
        inLimbo-core
)

set(INLIMBO_FRONTEND_TARGET inLimbo-fe-cmdline)
set(INLIMBO_FRONTEND_NAME   "cmdline")
