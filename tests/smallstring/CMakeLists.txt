# tests/smallstring/CMakeLists.txt

add_executable(smallstring_tests
  SmallStringAppend.test.cc
  SmallStringAssign.test.cc
  SmallStringCompare.test.cc
  SmallStringCTOR.test.cc
  SmallStringExtensionPath.test.cc
  SmallStringStrView.test.cc
  SmallStringSSO.test.cc
  SmallStringClear.test.cc
)

target_link_libraries(smallstring_tests
  PRIVATE
    ${GTEST_LIBS}
    inLimbo-core
)

target_include_directories(smallstring_tests
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include/inlimbo
)

include(GoogleTest)
gtest_discover_tests(smallstring_tests)
