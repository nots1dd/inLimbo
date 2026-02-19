#include "utils/string/SmallString.hpp"
#include "common.hpp"

#include <string>

using utils::string::SmallString;

constexpr int N = 100'000;
constexpr int APPENDS = 8;

auto main() -> int
{
  {
    Timer t;
    for (int i = 0; i < N; ++i)
    {
      SmallString s;
      for (int j = 0; j < APPENDS; ++j)
        s += "ab";
    }
    printResult("SmallString append (SSO)", t.elapsed_ms());
  }

  {
    Timer t;
    for (int i = 0; i < N; ++i)
    {
      std::string s;
      for (int j = 0; j < APPENDS; ++j)
        s += "ab";
    }
    printResult("std::string append (SSO)", t.elapsed_ms());
  }
}
