#pragma once

#include <chrono>
#include <iostream>

struct Timer
{
  using clock = std::chrono::high_resolution_clock;

  clock::time_point start;

  Timer() : start(clock::now()) {}

  [[nodiscard]] auto elapsed_ms() const -> double
  {
    return std::chrono::duration<double, std::milli>(
             clock::now() - start)
      .count();
  }
};

inline void printResult(const char* name, double ms)
{
  std::cout << std::setw(20) << name
            << " : " << ms << " ms\n";
}
