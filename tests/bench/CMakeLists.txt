# tests/bench/CMakeLists.txt

set(BENCHES
  smallstring_ctor
  smallstring_append_small
  smallstring_append_large
  smallstring_append_int
  smallstring_compare
  smallstring_path
)

foreach(bench ${BENCHES})
  add_executable(bench_${bench} bench_${bench}.cc)

  target_link_libraries(bench_${bench}
    PRIVATE inLimbo-core
  )

  target_compile_options(bench_${bench}
    PRIVATE -O3 -DNDEBUG
  )
endforeach()
