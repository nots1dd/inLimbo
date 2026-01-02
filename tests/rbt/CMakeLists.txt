# tests/rbt/CMakeLists.txt

add_executable(rbt_tests
  rbt_insert.test.cc
  rbt_invariants.test.cc
  rbt_erase.test.cc
  rbt_iterator.test.cc
  rbt_fuzz.test.cc
)

target_link_libraries(rbt_tests
  PRIVATE
    ${GTEST_LIBS}
    inLimbo-core
)

target_include_directories(rbt_tests
  PRIVATE
    ${PROJECT_SOURCE_DIR}/include/inlimbo
)

include(GoogleTest)
gtest_discover_tests(rbt_tests)
