#include "utils/string/SmallString.hpp"
#include "common.hpp"

#include <string>

using utils::string::SmallString;

constexpr int N = 100'000;

auto main() -> int
{
  {
    SmallString path("/music/album/song.flac");

    Timer t;
    for (int i = 0; i < N; ++i)
    {
      auto ext  = path.extension();
      auto name = path.filename();
      asm volatile("" ::: "memory");
    }
    printResult("SmallString path helpers", t.elapsed_ms());
  }

  {
    std::string path("/music/album/song.flac");

    Timer t;
    for (int i = 0; i < N; ++i)
    {
      auto pos  = path.find_last_of('.');
      auto ext  = (pos != std::string::npos) ? path.substr(pos) : std::string{};
      auto name = path.substr(path.find_last_of("/\\") + 1);
      asm volatile("" ::: "memory");
    }
    printResult("std::string path helpers", t.elapsed_ms());
  }
}
