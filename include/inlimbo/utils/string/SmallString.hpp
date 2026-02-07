#pragma once

#include <charconv>
#include <cstdint>
#include <cstring>
#include <string_view>

// A simple small-string optimization (SSO) string class.
// Supports up to 31 characters in SSO mode, otherwise allocates on the heap.
//
// Features:
// - SSO for small strings (up to 31 characters).
// - Append operators for various types (strings, chars, integrals, floating-point, bools, enums,
// pointers).
// - Automatic resizing and memory management.
// - Conversion to std::string_view for easy interop.
// - Comparison operators with std::string_view.
//
// When to use:
// - When you need a mutable string with frequent appends.
// - When you want to avoid heap allocations for small strings.
//
// Limitations:
// - Not thread-safe.
// - Designed for performance with a focus on common use cases.
// - Obviously not for long strings or heavy string manipulations. (like unicode, etc.)
//

namespace utils::string
{

class SmallString
{
public:
  static constexpr uint32_t m_ssoCAPACITY = 31;

  constexpr SmallString() noexcept { init_sso(); }

  SmallString(const char* s)
  {
    init_sso();
    if (s)
      append(std::string_view{s});
  }

  constexpr SmallString(char c) noexcept
  {
    init_sso();
    if (c)
      append_char(c);
  }

  SmallString(std::string_view sv)
  {
    init_sso();
    append(sv);
  }

  constexpr SmallString(const SmallString& other) { copy_from(other); }

  constexpr SmallString(SmallString&& other) noexcept { move_from(other); }

  constexpr ~SmallString() { free_heap(); }

  constexpr auto operator=(const SmallString& other) -> SmallString&
  {
    if (this != &other)
    {
      free_heap();
      copy_from(other);
    }
    return *this;
  }

  constexpr auto operator=(SmallString&& other) noexcept -> SmallString&
  {
    if (this != &other)
    {
      free_heap();
      move_from(other);
    }
    return *this;
  }

  // ---------------- Append basic ----------------
  auto operator+=(std::string_view sv) -> SmallString&
  {
    append(sv);
    return *this;
  }

  auto operator+=(const char* s) -> SmallString&
  {
    if (s)
    {
      append(std::string_view{s});
    }
    return *this;
  }

  constexpr auto operator+=(char c) -> SmallString&
  {
    append_char(c);
    return *this;
  }

  // ---------------- Append integrals ----------------
  template <typename T>
  constexpr auto operator+=(T v) -> SmallString&
    requires(std::is_integral_v<T> && !std::is_same_v<T, bool>)
  {
    append_integral(v);
    return *this;
  }

  // ---------------- Append floating ----------------
  template <typename T>
  constexpr auto operator+=(T v) -> SmallString&
    requires std::is_floating_point_v<T>
  {
    append_floating(v);
    return *this;
  }

  // ---------------- Append bool ----------------
  auto operator+=(bool v) -> SmallString&
  {
    append(v ? "true" : "false");
    return *this;
  }

  // ---------------- Append enum ----------------
  template <typename E>
  constexpr auto operator+=(E e) -> SmallString&
    requires std::is_enum_v<E>
  {
    using U = std::underlying_type_t<E>;
    append_integral(static_cast<U>(e));
    return *this;
  }

  // ---------------- Append pointer ----------------
  auto operator+=(const void* p) -> SmallString&
  {
    append_pointer(p);
    return *this;
  }

  // ---------------- Core append ----------------
  void append(std::string_view sv)
  {
    ensure_capacity(m_size + static_cast<uint32_t>(sv.size()));
    std::memcpy(m_data + m_size, sv.data(), sv.size());
    m_size += static_cast<uint32_t>(sv.size());
    m_data[m_size] = '\0';
  }

  constexpr void append_char(char c)
  {
    ensure_capacity(m_size + 1);
    m_data[m_size++] = c;
    m_data[m_size]   = '\0';
  }

  // ---------------- Explicit allocation ----------------
  constexpr void reserve(uint32_t cap) { ensure_capacity(cap); }

  // ---------------- Access ----------------
  [[nodiscard]] constexpr auto c_str() const noexcept -> const char* { return m_data; }
  constexpr auto               data() noexcept -> char* { return m_data; }

  [[nodiscard]] constexpr auto size() const noexcept -> uint32_t { return m_size; }
  [[nodiscard]] constexpr auto capacity() const noexcept -> uint32_t { return m_capacity; }
  [[nodiscard]] constexpr auto empty() const noexcept -> bool { return m_size == 0; }

  [[nodiscard]] constexpr auto is_sso() const noexcept -> bool { return m_data == m_sso; }

  constexpr void clear() noexcept
  {
    m_size    = 0;
    m_data[0] = '\0';
  }

  // ---------------- string_view interop ----------------
  [[nodiscard]] constexpr auto view() const noexcept -> std::string_view
  {
    return {m_data, m_size};
  }

  constexpr operator std::string_view() const noexcept { return view(); }

  // ---------------- Comparisons ----------------

  // SMALL STRING VS STRING_VIEW
  friend constexpr auto operator==(const SmallString& a, std::string_view b) noexcept -> bool
  {
    return a.view() == b;
  }

  friend constexpr auto operator!=(const SmallString& a, std::string_view b) noexcept -> bool
  {
    return !(a == b);
  }

  // STRING_VIEW VS SMALL STRING
  friend constexpr auto operator==(const SmallString& a, const SmallString& b) noexcept -> bool
  {
    return a.view() == b.view();
  }

  friend constexpr auto operator!=(const SmallString& a, const SmallString& b) noexcept -> bool
  {
    return !(a == b);
  }

  // SMALL STRING VS C-STRING
  friend constexpr auto operator==(const SmallString& a, const char* b) noexcept -> bool
  {
    if (!b)
      return false;
    return a.view() == std::string_view{b};
  }

  friend constexpr auto operator!=(const SmallString& a, const char* b) noexcept -> bool
  {
    return !(a == b);
  }

  [[nodiscard]] auto extension() const -> SmallString
  {
    SmallString out;

    if (m_size == 0)
      return out;

    // Find last path separator
    int last_sep = -1;
    for (uint32_t i = 0; i < m_size; ++i)
    {
      if (m_data[i] == '/' || m_data[i] == '\\')
        last_sep = static_cast<int>(i);
    }

    // Find last dot after last separator
    int last_dot = -1;
    for (uint32_t i = last_sep + 1; i < m_size; ++i)
    {
      if (m_data[i] == '.')
        last_dot = static_cast<int>(i);
    }

    // No extension or dot is first character of filename
    if (last_dot <= last_sep + 0)
      return out;

    out.append({m_data + last_dot, m_size - static_cast<uint32_t>(last_dot)});
    return out;
  }

  // Returns the filename component (last path segment).
  // Examples:
  //   "/a/b/c.txt"   -> "c.txt"
  //   "c.txt"        -> "c.txt"
  //   "/a/b/"        -> ""
  //   "C:\\a\\b\\x"  -> "x"
  [[nodiscard]] auto filename() const -> SmallString
  {
    SmallString out;

    if (empty())
      return out;

    // IMPORTANT: directory path == no filename
    if (m_data[m_size - 1] == '/' || m_data[m_size - 1] == '\\')
      return out;

    // Find last path separator
    int pos = static_cast<int>(m_size);
    while (pos > 0 && m_data[pos - 1] != '/' && m_data[pos - 1] != '\\')
      --pos;

    out.append({m_data + pos, static_cast<size_t>(m_size - pos)});
    return out;
  }

private:
  // ---------------- Storage ----------------
  char*    m_data;
  uint32_t m_size;
  uint32_t m_capacity;
  alignas(8) char m_sso[m_ssoCAPACITY + 1];

  // ---------------- Internals ----------------
  constexpr void init_sso() noexcept
  {
    m_data     = m_sso;
    m_size     = 0;
    m_capacity = m_ssoCAPACITY;
    m_sso[0]   = '\0';
  }

  constexpr void ensure_capacity(uint32_t needed)
  {
    if (needed <= m_capacity)
      return;

    uint32_t new_cap = m_capacity * 2;
    if (new_cap < needed)
      new_cap = needed;

    char* new_buf = new char[new_cap + 1];
    std::memcpy(new_buf, m_data, m_size + 1);

    free_heap();
    m_data     = new_buf;
    m_capacity = new_cap;
  }

  constexpr void free_heap() noexcept
  {
    if (!is_sso())
    {
      delete[] m_data;
    }
  }

  constexpr void copy_from(const SmallString& other)
  {
    if (other.m_size <= m_ssoCAPACITY)
    {
      init_sso();
      std::memcpy(m_sso, other.m_data, other.m_size + 1);
    }
    else
    {
      m_data = new char[other.m_capacity + 1];
      std::memcpy(m_data, other.m_data, other.m_size + 1);
      m_capacity = other.m_capacity;
    }
    m_size = other.m_size;
  }

  constexpr void move_from(SmallString& other) noexcept
  {
    if (other.is_sso())
    {
      init_sso();
      std::memcpy(m_sso, other.m_sso, other.m_size + 1);
    }
    else
    {
      m_data     = other.m_data;
      m_capacity = other.m_capacity;
      other.init_sso();
    }
    m_size = other.m_size;
    other.clear();
  }

  // ---------------- Numeric helpers ----------------
  template <typename T>
  void append_integral(T value)
  {
    char buf[std::numeric_limits<T>::digits10 + 3];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value);
    if (ec == std::errc{})
    {
      append({buf, static_cast<size_t>(ptr - buf)});
    }
  }

  template <typename T>
  void append_floating(T value)
  {
    char buf[64];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value, std::chars_format::general);
    if (ec == std::errc{})
    {
      append({buf, static_cast<size_t>(ptr - buf)});
    }
  }

  void append_pointer(const void* p)
  {
    append("0x");
    auto v = reinterpret_cast<uintptr_t>(p);

    char buf[2 * sizeof(void*) + 1];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v, 16);
    if (ec == std::errc{})
    {
      append({buf, static_cast<size_t>(ptr - buf)});
    }
  }
};

} // namespace utils::string
