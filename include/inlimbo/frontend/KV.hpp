#pragma once

namespace frontend
{

// this is for fetching the keybind's code
// and the name of the keybind itself.
//
// it has explicit methods to fetch the key
// and name along with some logical (maybe)
// operator overloads.
//
// check out frontend/cmdline/Structs.hpp
// or        frontend/raylib/Structs.hpp
// to see how it is being used to load config.

template <typename KeyT, typename NameT> struct KeyValueBase
{
  KeyT  key{};
  NameT name{};

  // Explicit key access
  [[nodiscard]] constexpr auto Key() const noexcept -> KeyT { return key; }

  // key access (operator overloading!!)
  [[nodiscard]] constexpr auto operator()() const noexcept -> KeyT { return key; }

  // Explicit name access
  [[nodiscard]] constexpr auto Name() const noexcept -> const NameT& { return name; }

  // name access (operator overloading!!)
  [[nodiscard]] constexpr auto operator->() const noexcept -> const NameT* { return &name; }

  // Key comparisons
  friend constexpr auto operator==(const KeyValueBase& kv, KeyT k) noexcept -> bool
  {
    return kv.key == k;
  }

  friend constexpr auto operator==(KeyT k, const KeyValueBase& kv) noexcept -> bool
  {
    return kv.key == k;
  }
};

} // namespace frontend
