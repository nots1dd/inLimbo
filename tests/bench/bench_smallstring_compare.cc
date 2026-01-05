#include "utils/string/SmallString.hpp"
#include "common.hpp"

#include <string>

using utils::string::SmallString;

constexpr int N = 100'000;

auto main() -> int
{
  {
    SmallString a("compare-me");
    SmallString b("compare-me");

    Timer t;
    for (int i = 0; i < N; ++i)
    {
      if (a == b)
        asm volatile("" ::: "memory");
    }
    printResult("SmallString compare", t.elapsed_ms());
  }

  {
    std::string a("compare-me");
    std::string b("compare-me");

    Timer t;
    for (int i = 0; i < N; ++i)
    {
      if (a == b)
        asm volatile("" ::: "memory");
    }
    printResult("std::string compare", t.elapsed_ms());
  }
}
