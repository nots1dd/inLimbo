#include "utils/string/SmallString.hpp"
#include "common.hpp"

#include <string>

using utils::string::SmallString;

constexpr int N = 100'000;

auto main() -> int
{
  {
    Timer t;
    for (int i = 0; i < N; ++i)
    {
      SmallString s;
      for (int j = 0; j < 64; ++j)
        s += 'x';
    }
    printResult("SmallString append (heap)", t.elapsed_ms());
  }

  {
    Timer t;
    for (int i = 0; i < N; ++i)
    {
      std::string s;
      for (int j = 0; j < 64; ++j)
        s += 'x';
    }
    printResult("std::string append (heap)", t.elapsed_ms());
  }
}
