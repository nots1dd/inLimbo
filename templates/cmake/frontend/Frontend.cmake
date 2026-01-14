# ===========================================================
# Dummy frontend
# ===========================================================

# These values have to be changed according to your plugin name.
# Ex: If my plugin name was cmdline, I would write:
# 
# set(INLIMBO_FRONTEND_TARGET      inLimbo-fe-cmdline)
# set(INLIMBO_FRONTEND_NAME        "cmdline")
# set(INLIMBO_FRONTEND_OUTPUT_NAME "inlimbo-frontend-dummy")
set(INLIMBO_FRONTEND_TARGET      inLimbo-fe-dummy)
set(INLIMBO_FRONTEND_NAME        "dummy")
set(INLIMBO_FRONTEND_OUTPUT_NAME "inlimbo-frontend-cmdline")

# Typically the file structure of a frontend would be like so:
#
# include/inlimbo/frontend/dummy/  --> Headers
# src/frontend/dummy/              --> Source files
# 
# The files that HAVE to be present at all costs are:
#   1. include/inlimbo/dummy/Interface.hpp (registering the plugin with the generic interface)
#   2. src/frontend/dummy/Interface.cc  (main entry point of the frontend plugin logic)
#   3. src/frontend/dummy/PluginShim.cc (to generate the frontend plugin)
# 
# Failing to abide by the above will most def cause compilation and frontend plugin errors as the 
# ABI is meant to be quite strict but powerful.
set(INLIMBO_FE_CMDLINE_SOURCES
  ${CMAKE_SOURCE_DIR}/src/frontend/dummy/Interface.cc
  ${CMAKE_SOURCE_DIR}/src/frontend/dummy/PluginShim.cc
)

# -------------------- NOTE ----------------------
# NOW THE REST OF THE FILE CAN BE IGNORED... (except for line 59)
# -------------------- NOTE ----------------------

add_library(${INLIMBO_FRONTEND_TARGET} SHARED
    ${INLIMBO_FE_CMDLINE_SOURCES}
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
    INLIMBO_FRONTEND_DUMMY # replace with INLIMBO_FRONTEND_<NAME> (ex: INLIMBO_FRONTEND_CMDLINE if frontend is cmdline)
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
            COMMENT ">> Deploying '${INLIMBO_FRONTEND_NAME}' frontend plugin to ${INLIMBO_USER_FRONTEND_DIR}"
)

list(APPEND INLIMBO_FRONTEND_NAMES "${INLIMBO_FRONTEND_NAME}")
