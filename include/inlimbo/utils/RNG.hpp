#pragma once

#include "InLimbo-Types.hpp"
#include <random>
#include <type_traits>

namespace utils
{

class RNG
{
public:
  template <typename Int>
  static auto intInRange(Int min, Int max) -> Int
    requires(std::is_integral_v<Int> && !std::is_same_v<Int, bool>)
  {
    if (min > max)
    {
      auto tmp = min;
      min      = max;
      max      = tmp;
    }

    std::uniform_int_distribution<Int> dist(min, max);
    return dist(engine());
  }

  template <typename Float>
  static auto floatInRange(Float min, Float max) -> Float
    requires(std::is_floating_point_v<Float>)
  {
    if (min > max)
    {
      auto tmp = min;
      min      = max;
      max      = tmp;
    }

    std::uniform_real_distribution<Float> dist(min, max);
    return dist(engine());
  }

  static auto u32(ui32 min, ui32 max) -> ui32 { return intInRange(min, max); }
  static auto i32(i32 min, i32 max) -> i32 { return intInRange(min, max); }
  static auto f32(float min, float max) -> float { return floatInRange(min, max); }
  static auto f64(double min, double max) -> double { return floatInRange(min, max); }

private:
  static auto engine() -> std::mt19937&
  {
    // Thread-local engine so multiple threads won't fight over a lock
    // and each thread gets some randomness ig.
    static thread_local std::mt19937 rng{std::random_device{}()};
    return rng;
  }
};

} // namespace utils
