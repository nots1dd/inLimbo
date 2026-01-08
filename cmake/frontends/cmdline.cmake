# ===========================================================
# Cmdline frontend
# ===========================================================

set(INLIMBO_FRONTEND_TARGET inLimbo-fe-cmdline)
set(INLIMBO_FRONTEND_NAME   "cmdline")

set(INLIMBO_FE_CMDLINE_SOURCES
  ${CMAKE_SOURCE_DIR}/src/frontend/cmdline/Interface.cc
  ${CMAKE_SOURCE_DIR}/src/frontend/cmdline/PluginShim.cc
)

add_library(${INLIMBO_FRONTEND_TARGET} SHARED
    ${INLIMBO_FE_CMDLINE_SOURCES}
)

set_target_properties(${INLIMBO_FRONTEND_TARGET} PROPERTIES
    OUTPUT_NAME "inlimbo-frontend-cmdline"
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
        INLIMBO_FRONTEND_CMDLINE
        $<$<CONFIG:Debug>:INLIMBO_DEBUG_BUILD>
)

target_link_libraries(${INLIMBO_FRONTEND_TARGET}
    PRIVATE
        inLimbo-core
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
    COMMENT ">> Deploying frontend plugin to ${INLIMBO_USER_FRONTEND_DIR}"
)

list(APPEND INLIMBO_FRONTEND_NAMES "${INLIMBO_FRONTEND_NAME}")
