#include "utils/string/SmallString.hpp"
#include "common.hpp"

#include <string>
#include <vector>

using utils::string::SmallString;

constexpr int N = 100'000;

auto main() -> int
{
  std::vector<int> values(N);
  for (int i = 0; i < N; ++i)
    values[i] = i;

  {
    Timer t;
    for (int v : values)
    {
      SmallString s("value=");
      s += v;
    }
    printResult("SmallString append int", t.elapsed_ms());
  }

  {
    Timer t;
    for (int v : values)
    {
      std::string s("value=");
      s += std::to_string(v);
    }
    printResult("std::string append int", t.elapsed_ms());
  }
}
