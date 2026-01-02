# tests/bench/CMakeLists.txt

set(BENCHES
  rbt_insert
  rbt_iterate
  rbt_erase
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
