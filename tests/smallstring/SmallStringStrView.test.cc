#include <gtest/gtest.h>

#include "utils/string/SmallString.hpp"

using utils::string::SmallString;

// ------------------------------------------------------------
// string_view interop
// ------------------------------------------------------------

TEST(SmallStringInterop, StringViewConversion)
{
  SmallString s("hello");
  std::string_view sv = s;
  EXPECT_EQ(sv, "hello");
}
