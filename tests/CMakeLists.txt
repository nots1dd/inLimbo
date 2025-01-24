# Find all subdirectories in tests and add them
file(GLOB TEST_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} */)

foreach(test_dir ${TEST_DIRS})
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}/CMakeLists.txt)
        set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${test_dir}/build)
        add_subdirectory(${test_dir})
    endif()
endforeach()
