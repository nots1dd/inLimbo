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
      SmallString s("abc");
      (void)s;
    }
    printResult("SmallString ctor (SSO)", t.elapsed_ms());
  }

  {
    Timer t;
    for (int i = 0; i < N; ++i)
    {
      std::string s("abc");
      (void)s;
    }
    printResult("std::string ctor (SSO)", t.elapsed_ms());
  }
}
