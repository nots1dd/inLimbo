cmake_minimum_required(VERSION 3.16)
project(MiniAudioPlayerTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

set(TEST_SOURCES
  MiniAudioPlayerTestLinux.cpp
)

set(SRC_PATH ../../src/music/)

set(SRC_FILES
    ${SRC_PATH}/audio_playback.hpp
)

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${SRC_PATH})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

add_executable(${PROJECT_NAME}
    ${TEST_SOURCES}
    ${SRC_FILES}
)

target_link_libraries(${PROJECT_NAME}
    ${GTEST_BOTH_LIBRARIES}
    pthread
)

include(CTest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
